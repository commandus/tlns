#include "message-task-dispatcher.h"

#include <algorithm>
#include <functional>
#include <csignal>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <iostream>

#include "lorawan/lorawan-error.h"
#include "lorawan/proto/gw/basic-udp.h"
#include "lorawan/lorawan-string.h"

#define DEF_TIMEOUT_SECONDS 3
#define DEF_WAIT_QUIT_SECONDS 1

TaskSocket::TaskSocket(
    in_addr_t aAddr,
    uint16_t aPort,
    TaskProc aCb
)
    : sock(-1), addr(aAddr), port(aPort), lastError(CODE_OK), cb(aCb)
{

}

TaskSocket::~TaskSocket()
{
    closeSocket();
}

SOCKET TaskSocket::openUDPSocket()
{
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        lastError = ERR_CODE_SOCKET_CREATE;
        return -1;
    }
    // Allow socket descriptor to be reuseable
    int on = 1;
    int rc = setsockopt(sock, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
    if (rc < 0) {
        close(sock);
        sock = -1;
        lastError = ERR_CODE_SOCKET_OPEN;
        return -1;
    }
    // Set socket to be nonblocking
    rc = ioctl(sock, FIONBIO, (char *)&on);
    if (rc < 0) {
        close(sock);
        sock = -1;
        lastError = ERR_CODE_SOCKET_OPEN;
        return -1;
    }

    // Bind the socket
    struct sockaddr_in saddr {};
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(addr); // inet_pton(AF_INET, addr.c_str(), &(saddr.sin_addr));
    saddr.sin_port = htons(port);
    rc = bind(sock, (struct sockaddr *) &saddr, sizeof(saddr));
    if (rc < 0) {
        close(sock);
        sock = -1;
        lastError = ERR_CODE_SOCKET_BIND;
        return -1;
    }
    lastError = CODE_OK;
    return sock;
}

void TaskSocket::closeSocket()
{
    if (sock >= 0)
        close(sock);
    sock = -1;
}

//----------------------------------------------------------------------------------

MessageTaskDispatcher::MessageTaskDispatcher()
    : clientControlSocket(-1), taskResponse(nullptr), thread(nullptr),
      parser(new GatewayBasicUdpProtocol(this)), queue(nullptr), controlSocket(nullptr), running(false)
{

}

MessageTaskDispatcher::MessageTaskDispatcher(
    const MessageTaskDispatcher &value
)
    : clientControlSocket(-1), taskResponse(value.taskResponse), thread(value.thread),
      parser(value.parser), queue(value.queue), controlSocket(nullptr), running(value.running)
{
}

MessageTaskDispatcher::~MessageTaskDispatcher()
{
    stop();
    clearSockets();
    delete parser;
}

void MessageTaskDispatcher::setQueue(
    MessageQueue *aQueue
)
{
    queue = aQueue;
    queue->setDispatcher(this);
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

void MessageTaskDispatcher::send(
    const void *cmd,
    size_t size
)
{
    if (clientControlSocket < 0 || !controlSocket)
        return;
    sockaddr_in destination {
        .sin_family = AF_INET,
        .sin_port = htons(controlSocket->port)
    };
    destination.sin_addr.s_addr = htonl(controlSocket->addr);
    sendto(clientControlSocket, cmd, size, 0, (const sockaddr *) &destination, sizeof(destination));
}

void MessageTaskDispatcher::send(
    const std::string & cmd
)
{
    send(cmd.c_str(), cmd.size());
}

void MessageTaskDispatcher::send(
    char cmd
)
{
    char c = 'q';
    send(&c, 1);
}

bool MessageTaskDispatcher::start()
{
    if (running)
        return true;
    thread = new std::thread(std::bind(&MessageTaskDispatcher::runner, this));
    thread->detach();
    return true;
}

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
        if (s->lastError || s->openUDPSocket() < 0) {
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

int MessageTaskDispatcher::runner()
{
    if (sockets.empty())
        return ERR_CODE_PARAM_INVALID;
    if (!openSockets()) {
        closeSockets();
        return ERR_CODE_SOCKET_CREATE;
    }

    struct timeval timeout {};

    // Initialize the master fd_set
    fd_set master_set;
    // , working_set;
    FD_ZERO(&master_set);

    // sort sockets ascendant
    std::sort(sockets.begin(), sockets.end(),
        [] (TaskSocket* a, TaskSocket* b) {
        return a->sock < b->sock;
    });
    //
    SOCKET maxFD1 = sockets.back()->sock + 1;

    for (auto s : sockets) {
        FD_SET(s->sock, &master_set);
    }

    char buffer[4096];

    running = true;
    clientControlSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientControlSocket < 0)
        return ERR_CODE_PARAM_INVALID;

    while (running) {
        fd_set working_set;
        // Copy the master fd_set over to the working fd_set
        memcpy(&working_set, &master_set, sizeof(master_set));
        // Initialize the timeval struct
        timeout = { .tv_sec = DEF_TIMEOUT_SECONDS, .tv_usec = 0 };
        int rc = select(maxFD1, &working_set, nullptr, nullptr, &timeout);
        if (rc < 0) { // select error
            break;
        }
        if (rc == 0) { // select() timed out.
            continue;
        }
        // get timestamp
        TASK_TIME receivedTime = std::chrono::system_clock::now();
        for (auto s : sockets) {
            if (FD_ISSET(s->sock, &working_set)) {
                struct sockaddr srcAddr;
                socklen_t srcAddrLen = sizeof(srcAddr);
                ssize_t sz = recvfrom(s->sock, buffer, sizeof(buffer), 0, &srcAddr, &srcAddrLen);
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
                        case SIZE_DEVADDR:
                        {
                            DEVADDR *a = (DEVADDR *) buffer;
                            // process message queue
                            MessageQueueItem *item = queue->findByDevAddr(a);
                        }
                            break;
                        default: {
                            int r = s->cb(this, s, &srcAddr, receivedTime, buffer, sz);
                            if (r < 0) {
                                // inform
                            }
                        }
                    }
                }
            }
        }
    }
    closeSockets();
    close(clientControlSocket);
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
    return sendto(taskSocket->sock, &ack, SIZE_SEMTECH_ACK, 0, &destAddr, destAddrLen);
}

// --------------------------------------------------------------

/**
 * Create control socket
 * @param dispatcher owner
 * @param addr socket address
 * @param port port number
 * @return socket, -1 if fail
 */
TaskSocket* createDumbControlSocket(
    MessageTaskDispatcher *dispatcher,
    in_addr_t addr,
    uint16_t port
)
{
    return new TaskSocket(addr, port, [] (
        MessageTaskDispatcher *dispatcher,
        TaskSocket *taskSocket,
        const struct sockaddr *gwAddr,
        TASK_TIME receivedTime,
        const char *buffer,
        size_t size
    ) {
        // Join request
        // JOIN_REQUEST_FRAME *f = (JOIN_REQUEST_FRAME *) buffer;
        // MessageQueueItem *item = dispatcher->queue->findByJoinRequest(f);

        // get gateway identifier first
        int r = dispatcher->parser->parse(buffer, size, receivedTime,
            [](MessageTaskDispatcher*dispatcher,  GwPushData &item) {
                std::cout << SEMTECH_PROTOCOL_METADATA_RX2string(item.rxMetadata) << std::endl;
                dispatcher->queue->put(item);
            }, nullptr, nullptr);
        return r;
    });
}
