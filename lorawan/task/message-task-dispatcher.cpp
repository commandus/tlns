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
#include "task-accepted-socket.h"

#define DEF_TIMEOUT_SECONDS 3
#define DEF_WAIT_QUIT_SECONDS 1

MessageTaskDispatcher::MessageTaskDispatcher()
    : controlSocket(nullptr), taskResponse(nullptr), thread(nullptr),
      parser(nullptr), regionalPlan(nullptr), running(false),
      onPushData(nullptr), onPullResp(nullptr), onTxPkAck(nullptr), onDestroy(nullptr)
{
    queue.setDispatcher(this);
}

MessageTaskDispatcher::MessageTaskDispatcher(
    const MessageTaskDispatcher &value
)
    : controlSocket(nullptr), taskResponse(value.taskResponse), thread(value.thread),
      parser(value.parser), regionalPlan(nullptr), queue(value.queue), running(value.running),
      onPushData(nullptr), onPullResp(nullptr), onTxPkAck(nullptr)
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
    return r;
}

void MessageTaskDispatcher::closeSockets()
{
    // close all sockets
    for (auto s : sockets) {
        s->closeSocket();
    }
}

void MessageTaskDispatcher::clearSockets()
{
    for (auto s : sockets) {
        delete s;
    }
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
        if (rc == 0)    // select() timed out.
            continue;
        // get timestamp
        TASK_TIME receivedTime = std::chrono::system_clock::now();

        std::vector<SOCKET> acceptedSockets;
        std::vector<TaskSocket*> removedSockets;

        for (auto s : sockets) {
            if (FD_ISSET(s->sock, &workingSocketSet)) {
                struct sockaddr srcAddr;
                socklen_t srcAddrLen = sizeof(srcAddr);
                ssize_t sz;
                if (s->accept == SA_REQUIRE) {
                    // TCP, UNIX sockets require create accept socket and create a new paired socket
                    SOCKET cfd = accept(s->sock, (struct sockaddr *) &srcAddr, &srcAddrLen);
                    if (cfd > 0)
                        acceptedSockets.push_back(cfd); // do not modify vector using iterator, do it after
                    continue;
                    // sz = read(cfd, buffer, sizeof(buffer));
                } else {
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
                            if (parser) {
                                int r = parser->parse(pr, buffer, sz, receivedTime);
                                if (r == CODE_OK) {
                                    switch (pr.tag) {
                                        case SEMTECH_GW_PUSH_DATA:
                                            pushData(pr.gwPushData);
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
                                if (r < 0) {
                                    // inform
                                }
                            }
                        }
                    }
                }
            }
        }
        // accept connections
        if (!acceptedSockets.empty()) {
            for (auto a = acceptedSockets.begin(); a != acceptedSockets.end(); a++) {
                sockets.push_back(new TaskAcceptedSocket(*a));
            }
            acceptedSockets.clear();
            maxFD1 = getMaxDescriptor1(masterReadSocketSet);
        }
        // delete broken connections
        if (!removedSockets.empty()) {
            for (auto a = removedSockets.begin(); a != removedSockets.end(); a++) {
                // get max socket
                auto f = std::find_if(sockets.begin(), sockets.end(), [a](const TaskSocket *v) {
                    return (*a == v);
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
    if (packetSize < SIZE_SEMTECH_ACK)
        return ERR_CODE_SEND_ACK;
    SEMTECH_ACK ack;
    memmove(&ack, packet, SIZE_SEMTECH_ACK);
    if (ack.version != 2)
        return ERR_CODE_SEND_ACK;
    ack.tag++;
    return sendto(taskSocket->sock, (const char *)  &ack, SIZE_SEMTECH_ACK, 0, &destAddr, destAddrLen);
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
    GwPushData &pushData
) {
    queueMutex.lock();
    bool isNew = queue.put(pushData);
    queueMutex.unlock();
    // wake up
    // if (isNew) {
    auto a = pushData.rxData.getAddr();
    if (a)
        send(a, SIZE_DEVADDR);  // pass address of just added item
    if (pushData.needConfirmation()) {
        // send confirmation
        std::cout << "** NEED CONFIRMATION **" << std::endl;
        queueMutex.lock();
        bool isNew = queue.put(pushData);
        queueMutex.unlock();
    }
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
