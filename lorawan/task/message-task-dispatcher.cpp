#include "message-task-dispatcher.h"

#include <algorithm>
#include <functional>
#include <csignal>
#include "lorawan/task/task-platform.h"
#if defined(_MSC_VER)
#else
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
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
      parser(new GatewayBasicUdpProtocol(this)), running(false),
      onPushData(nullptr), onPullResp(nullptr), onTxPkAck(nullptr)
{
    queue.setDispatcher(this);
}

MessageTaskDispatcher::MessageTaskDispatcher(
    const MessageTaskDispatcher &value
)
    : controlSocket(nullptr), taskResponse(value.taskResponse), thread(value.thread),
      parser(value.parser), queue(value.queue), running(value.running),
      onPushData(nullptr), onPullResp(nullptr), onTxPkAck(nullptr)
{
}

MessageTaskDispatcher::~MessageTaskDispatcher()
{
    stop();
    clearSockets();
    delete parser;
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
bool MessageTaskDispatcher::start()
{
    if (running)
        return true;
    running = true;
    thread = new std::thread(std::bind(&MessageTaskDispatcher::runner, this));
    thread->detach();
    return true;
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
    while (running && loopExit.wait_for(lock, std::chrono::seconds(DEF_WAIT_QUIT_SECONDS)) == std::cv_status::timeout) {
        // try wake-up select() if UDP packet is missed
        send(&q, 1);
    }
    // free up resources
    delete thread;
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
    fd_set &retVal
)
{
    FD_ZERO(&retVal);
    // sort sockets ascendant
    std::sort(sockets.begin(), sockets.end(),
        [] (TaskSocket* a, TaskSocket* b) {
            return a->sock < b->sock;
        }
    );
    SOCKET maxFD1 = sockets.back()->sock + 1;
    for (auto s : sockets) {
        FD_SET(s->sock, &retVal);
    }
    return maxFD1;
}

/**
 * Dispatcher main loop
 * @return 0 if success or negative error code
 */
int MessageTaskDispatcher::runner()
{
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
    fd_set masterSocketSet;
    SOCKET maxFD1 = getMaxDescriptor1(masterSocketSet);
    char buffer[4096];

    ParseResult pr;
    while (running) {
        fd_set workingSocketSet;
        // Copy the master fd_set over to the working fd_set
        memcpy(&workingSocketSet, &masterSocketSet, sizeof(masterSocketSet));
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

        std::vector<SOCKET> acceptedSockets(4);

        for (auto s : sockets) {
            if (FD_ISSET(s->sock, &workingSocketSet)) {
                struct sockaddr srcAddr;
                socklen_t srcAddrLen = sizeof(srcAddr);
                ssize_t sz;
                if (s->accept == SA_REQUIRE) {
                    SOCKET cfd = accept(s->sock, (struct sockaddr *) &srcAddr, &srcAddrLen);
                    if (cfd > 0) {
                        acceptedSockets.push_back(cfd);
                    }
                    continue;
                    // sz = read(cfd, buffer, sizeof(buffer));
                } else {
                    sz = read(s->sock, buffer, sizeof(buffer));
                }
                if (sz < 0) {
                    std::cerr << ERR_MESSAGE  << errno << ": " << strerror(errno)
                        << " socket " << s->sock
                        << std::endl;
                    continue;
                }
                if (sz > 0) {
                    // send ACK immediately
                    /*
                    if (sendACK(s, srcAddr, srcAddrLen, buffer, sz) > 0) {
                        // inform
                    }
                     */
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
                        case SIZE_DEVADDR:
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
                            break;
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
        if (acceptedSockets.size()) {
            for (auto a = acceptedSockets.begin(); a != acceptedSockets.end(); a++) {
                // get max socket
                sockets.push_back(new TaskAcceptedSocket(*a));
            }
            maxFD1 = getMaxDescriptor1(masterSocketSet);
        }
    }
    closeSockets();
    running = false;
    loopExit.notify_all();
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

void MessageTaskDispatcher::setControlSocket(
    TaskSocket *socket
)
{
    auto f = std::find_if(sockets.begin(), sockets.end(), [socket](const TaskSocket *v) {
        return (socket == v);
    });
    if (f == sockets.end())
        return;
    controlSocket = *f;
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
        send(a, SIZE_DEVADDR);
}
