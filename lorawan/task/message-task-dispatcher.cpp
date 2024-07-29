#include "message-task-dispatcher.h"

#include <algorithm>
#include <functional>
#include <csignal>
#if defined(_MSC_VER)
#else
#include <sys/select.h>
#include <sys/socket.h>
#endif
#include <cstring>
#include <iostream>

#include "lorawan/lorawan-error.h"
#include "lorawan/proto/gw/basic-udp.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-msg.h"
#include "lorawan/task/task-accepted-socket.h"
#include "lorawan/lorawan-date.h"

#define DEF_TIMEOUT_SECONDS 3
#define DEF_WAIT_QUIT_SECONDS 1
#define MAX_ACK_SIZE    8
#define MIN_TIMER_IN_MICROSECONDS   9000

MessageTaskDispatcher::MessageTaskDispatcher()
    : controlSocket(nullptr), timerSocket(new TaskTimerSocket), taskResponse(nullptr), thread(nullptr),
      parser(nullptr), regionalPlan(nullptr), identity(nullptr), running(false),
      onReceiveRawData(nullptr), onPushData(nullptr), onPullResp(nullptr), onTxPkAck(nullptr), onDestroy(nullptr),
      onError(nullptr)
{
    queue.setDispatcher(this);
    sockets.push_back(timerSocket);
}

MessageTaskDispatcher::MessageTaskDispatcher(
    const MessageTaskDispatcher &value
)
    : controlSocket(value.controlSocket), timerSocket(value.timerSocket), taskResponse(value.taskResponse),
        thread(value.thread), parser(value.parser), regionalPlan(value.regionalPlan), identity(value.identity),
        queue(value.queue), running(value.running), onReceiveRawData(value.onReceiveRawData),
        onPushData(value.onPushData), onPullResp(value.onPullResp), onTxPkAck(value.onTxPkAck),
        onDestroy(value.onDestroy), onError(nullptr)
{
}

MessageTaskDispatcher::~MessageTaskDispatcher()
{
    stop();
    clearSockets();
    if (onDestroy)
        onDestroy(this);
}

void MessageTaskDispatcher::response(
    MessageQueueItem *item
)
{

}

void MessageTaskDispatcher::setResponse(
    TaskResponse *value
)
{
    taskResponse = value;
}

/**
 * Send buffer to client control socket
 * @param cmd buffer
 * @param size size
 */
void MessageTaskDispatcher::send(
    const void *cmd,
    size_t size
)
{
    if (controlSocket)
        write(controlSocket->sock, (const char *) cmd, size);
}

/**
 * Send string to client control socket
 * @param cmd String size must be less 4K
 */
void MessageTaskDispatcher::send(
    const std::string &cmd
)
{
    send(cmd.c_str(), cmd.size());
}

void MessageTaskDispatcher::send(
    char cmd
)
{
    send(&cmd, 1);
}

/**
 * Run dispatcher runner in separate thread.
 * If dispatcher running already, return true.
 * @return true always
 */
void MessageTaskDispatcher::start()
{
    if (running)
        return;
    running = true; // force running because thread start with delay
    thread = new std::thread(std::bind(&MessageTaskDispatcher::run, this));
    thread->detach();
}

/**
 * Stop dispatcher runner.
 * Set stop request flag and send wake-up message.
 * Then wait until dispatcher loop finished then destroy dispatcher thread.
 */
void MessageTaskDispatcher::stop()
{
    if (!running)
        return;
    // wake-up select()
    char q = 'q';
    send(&q, 1);

    // wait until thread finish
    std::mutex m;
    std::unique_lock<std::mutex> lock(m);
    if (thread) {
        while (running) {
            // try wake-up select() if UDP packet is missed
            send(&q, 1);
        }
        // free up resources
        delete thread;
        thread = nullptr;
    }
}

bool MessageTaskDispatcher::openSockets()
{
    bool r = true;
    for (auto s : sockets) {
        if (s->lastError || s->openSocket() < 0) {
            r = false;
            break;
        }
    }
    if (timerSocket)
        timerSocket->openSocket();
    return r;
}

void MessageTaskDispatcher::closeSockets()
{
    // close all sockets
    for (auto s : sockets) {
        s->closeSocket();
    }
    if (timerSocket) {
        timerSocket->closeSocket();
        timerSocket = nullptr;
    }
}

void MessageTaskDispatcher::clearSockets()
{
    for (auto s : sockets)
        delete s;
    // clear container
    sockets.clear();
}


int MessageTaskDispatcher::getMaxDescriptor1(
    fd_set &retValReadSet
)
{
    FD_ZERO(&retValReadSet);
    // sort sockets ascendant
    std::sort(sockets.begin(), sockets.end(),
        [] (TaskSocket* a, TaskSocket* b) {
            return a->sock < b->sock;
        }
    );
    SOCKET maxFD1 = sockets.back()->sock + 1;
    for (auto s : sockets) {
        FD_SET(s->sock, &retValReadSet);
    }
    return maxFD1;
}

/**
 * Dispatcher main loop
 * @return 0 if success or negative error code
 */
int MessageTaskDispatcher::run()
{
    running = true;
    if (sockets.empty()) {
        running = false;
        return ERR_CODE_PARAM_INVALID;
    }
    if (!openSockets()) {
        closeSockets();
        running = false;
        return ERR_CODE_SOCKET_CREATE;
    }
    struct timeval timeout {};

    // Initialize the master fd_set
    fd_set masterReadSocketSet;
    SOCKET maxFD1 = getMaxDescriptor1(masterReadSocketSet);
    char buffer[4096];

    ParseResult pr;
    while (running) {
        fd_set workingSocketSet;
        // Copy the master fd_set over to the working fd_set
        memcpy(&workingSocketSet, &masterReadSocketSet, sizeof(masterReadSocketSet));
        // Initialize the timeval struct
        timeout.tv_sec = DEF_TIMEOUT_SECONDS;
        timeout.tv_usec = 0;
        int rc = select(maxFD1, &workingSocketSet, nullptr, nullptr, &timeout);
        if (rc < 0)     // select error
            break;

        // get timestamp
        TASK_TIME receivedTime = std::chrono::system_clock::now();

        if (rc == 0) {   // select() timed out.
            cleanupOldMessages(receivedTime);
            continue;
        }

        std::vector<SOCKET> acceptedSockets;
        std::vector<TaskSocket*> removedSockets;

        struct sockaddr srcAddr;
        socklen_t srcAddrLen = sizeof(srcAddr);

        // read socket(s)
        for (auto s : sockets) {
            if (!FD_ISSET(s->sock, &workingSocketSet))
                continue;
            ssize_t sz;
            switch (s->accept) {
                case SA_REQUIRE: {
                    // TCP, UNIX sockets require create accept socket and create a new paired socket
                    SOCKET cfd = accept(s->sock, (struct sockaddr *) &srcAddr, &srcAddrLen);
                    if (cfd > 0)
                        acceptedSockets.push_back(cfd); // do not modify vector using iterator, do it after
                    continue;
                }
                case SA_TIMER:
                    std::cout << "Timer event " << taskTime2string(receivedTime) << std::endl;
                    sz = read(s->sock, buffer, sizeof(buffer)); // 8 bytes, timer counter value
                    sendQueue(receivedTime, pr.token);
                    // set timer
                    if (isTimeProcessQueueOrSetTimer(receivedTime)) {
                        // nothing to do
                    }
                    cleanupOldMessages(receivedTime);
                    continue;
                case SA_EVENTFD:
                    // not used
                    sz = read(s->sock, buffer, sizeof(buffer));
                    break;
                default:
                    sz = recvfrom(s->sock, buffer, sizeof(buffer), 0, &srcAddr, &srcAddrLen);
            }
            if (sz < 0) {
                std::cerr << ERR_MESSAGE  << errno << ": " << strerror(errno)
                    << " socket " << s->sock
                    << std::endl;
                if (s->accept == SA_ACCEPTED) {
                    // close client connection
                    removedSockets.push_back(s); // do not modify vector using iterator, do it after
                }
                continue;
            }
            if (sz > 0) {
                // send ACK immediately
                if (sendACK(s, srcAddr, srcAddrLen, buffer, sz) > 0) {
                    // inform
                }
                switch (pr.tag) {
                    default:
                        if (isTimeProcessQueueOrSetTimer(receivedTime)) {
                            // try send again
                            sendQueue(receivedTime, pr.token);
                        }
                }
                switch (sz) {
                    case 1:
                    {
                        char *a = (char *) buffer;
                        switch (*a) {
                            case 'q':
                                running = false;
                                break;
                            default:
                                break;
                        }
                    }
                        break;
                    case SIZE_DEVADDR:      // something happens on device (by address)
                    {
                        auto *a = (DEVADDR *) buffer;
                        // process message queue
                        MessageQueueItem *item = queue.findByDevAddr(a);
                        if (item) {
                            if (onPushData)
                                onPushData(this, item);
                        }
                        break;
                    }
                    default: {
                        if (onReceiveRawData)
                            if (!onReceiveRawData(this, buffer, sz, receivedTime))
                                continue;
                        if (parser) {
                            int r = parser->parse(pr, buffer, sz, receivedTime);
                            if (r == CODE_OK) {
                                r = validateGatewayAddress(pr, s, srcAddr);
                                if (r == CODE_OK) {
                                    switch (pr.tag) {
                                        case SEMTECH_GW_PUSH_DATA:
                                            pushData(s, srcAddr, pr.gwPushData, receivedTime);
                                            break;
                                        case SEMTECH_GW_PULL_RESP:
                                            if (onPullResp)
                                                onPullResp(this, pr.gwPullResp);
                                            break;
                                        case SEMTECH_GW_TX_ACK:
                                            onTxPkAck(this, pr.code);
                                            break;
                                        default:
                                            break;
                                    }
                                }
                            }
                            if (r < 0) {
                                // inform
                            }
                        }
                    }
                }
            }
        }
        // accept connections
        if (!acceptedSockets.empty()) {
            for (int & acceptedSocket : acceptedSockets) {
                sockets.push_back(new TaskAcceptedSocket(acceptedSocket));
            }
            acceptedSockets.clear();
            maxFD1 = getMaxDescriptor1(masterReadSocketSet);
        }
        // delete broken connections
        if (!removedSockets.empty()) {
            for (auto & removedSocket : removedSockets) {
                // get max socket
                auto f = std::find_if(sockets.begin(), sockets.end(), [&removedSocket](const TaskSocket *v) {
                    return (removedSocket == v);
                });
                if (f != sockets.end())
                    sockets.erase(f);
            }
            removedSockets.clear();
            maxFD1 = getMaxDescriptor1(masterReadSocketSet);
        }
    }
    closeSockets();
    running = false;
    return CODE_OK;
}

ssize_t MessageTaskDispatcher::sendACK(
    const TaskSocket *taskSocket,
    const sockaddr &destAddr,
    socklen_t destAddrLen,
    const char *packet,
    ssize_t packetSize
) {
    char ack[MAX_ACK_SIZE];
    if (!this->parser)
        return ERR_CODE_SEND_ACK;
    ssize_t sz = this->parser->ack(ack, MAX_ACK_SIZE, packet, packetSize);
    if (sz <= 0)
        return sz;
    return sendto(taskSocket->sock, (const char *)  &ack, sz, 0, &destAddr, destAddrLen);
}

ssize_t MessageTaskDispatcher::sendConfirm(
    const TaskSocket *taskSocket,
    const sockaddr &destAddr,
    socklen_t destAddrLen
) {
    SEMTECH_ACK ack;
    return sendto(taskSocket->sock, (const char *)  &ack, SIZE_SEMTECH_ACK, 0, &destAddr, destAddrLen);
}

void MessageTaskDispatcher::setControlSocket(
    TaskSocket *socket
)
{
    auto f = std::find_if(sockets.begin(), sockets.end(), [socket](const TaskSocket *v) {
        return (socket == v);
    });
    if (f == sockets.end())
        sockets.push_back(socket);
    controlSocket = socket;
}

void MessageTaskDispatcher::pushData(
    const TaskSocket *taskSocket,
    const sockaddr &addr,
    GwPushData &pushData,
    const TASK_TIME &receivedTime
) {
    queueMutex.lock();
    bool isNew = queue.put(receivedTime, taskSocket, addr, pushData);
    queueMutex.unlock();
    // wake up
    // if (isNew) {
    auto a = pushData.rxData.getAddr();
    if (a)
        send(a, SIZE_DEVADDR);  // pass address of just added item
    if (pushData.needConfirmation())
        prepareSendConfirmation(a, addr, receivedTime);
}

void MessageTaskDispatcher::setParser(
    ProtoGwParser *aParser)
{
    parser = aParser;
}

void MessageTaskDispatcher::setRegionalParameterChannelPlan(
    const RegionalParameterChannelPlan *aRegionalPlan
)
{
    regionalPlan = aRegionalPlan;
}

void MessageTaskDispatcher::sendQueue(
    TASK_TIME now,
    uint8_t token
) {
    TimeAddr ta;
    while (queue.time2ResponseAddr.pop(ta, now)) {
        std::cout << "Pop and send from queue " << ta.toString() << "\n";
        auto m = queue.receivedMessages.find(ta.addr);
        if (m == queue.receivedMessages.end())
            continue;
        if (m->second.needConfirmation()) {
            ConfirmationMessage confirmationMessage(m->second.radioPacket, m->second.task);
            GatewayMetadata gwMetadata;
            if (m->second.getBestGatewayAddress(gwMetadata)) {
                char sb[512];
                auto sz = this->parser->makeMessage2Gateway(sb, sizeof(sb), confirmationMessage,
                    token, &gwMetadata.rx, regionalPlan);
                std::cout << "Send " << std::string(sb, sz)
                    << " to gateway " << gatewayId2str(gwMetadata.rx.gatewayId)
                    << " at " << sockaddr2string(&m->second.task.gatewayId.sockaddr)
                    << std::endl;
            }
        }
    }
}

bool MessageTaskDispatcher::isTimeProcessQueueOrSetTimer(
    TASK_TIME now
)
{
    auto d = queue.time2ResponseAddr.waitTimeForAllGatewaysInMicroseconds(now);
    if (d < 0)
        return false;

    if (d < MIN_TIMER_IN_MICROSECONDS)
        return true;

    now += std::chrono::microseconds(d);

    if (!timerSocket->setStartupTime(now)) {
        if (onError)
            onError(this, LOG_CRIT, "Set timer", errno, strerror(errno));
        return true;
    }
    return false;
}

/**
 * Put address to the queue and set timer to start send confirmation after 0.5s after first message
 * @param addr address
 * @param receivedTime receive time
 */
void MessageTaskDispatcher::prepareSendConfirmation(
    const DEVADDR *addr,
    const sockaddr &sockAddr,
    TASK_TIME receivedTime
)
{
    queue.time2ResponseAddr.push(*addr, receivedTime);
}

void MessageTaskDispatcher::cleanupOldMessages(
    TASK_TIME now
)
{
    queue.clearOldMessages(now - std::chrono::seconds(1));
}

int MessageTaskDispatcher::validateGatewayAddress(
    const ParseResult &parsedMsg,
    const TaskSocket *taskSocket,
    const sockaddr &srcSockAddr
)
{
    return CODE_OK;
}

void MessageTaskDispatcher::setIdentityClient(
    DirectClient *aIdentitClient
) {
    identity = aIdentitClient;
}
