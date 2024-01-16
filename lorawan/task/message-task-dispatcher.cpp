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

#define CONTROL_PORT    4242
#define DEF_TIMEOUT_SEC 3

TaskSocket::TaskSocket(
    in_addr_t aIntfType,
    uint16_t aPort,
    TaskProc aCb
)
    : sock(-1), intfType(aIntfType), port(aPort), lastError(CODE_OK), cb(aCb)
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
    saddr.sin_addr.s_addr = htonl(intfType); // inet_pton(AF_INET, addr.c_str(), &(saddr.sin_addr));
    saddr.sin_port = htons(CONTROL_PORT);
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
    : queue(nullptr), taskResponse(nullptr), thread(nullptr), running(false)
{

}

MessageTaskDispatcher::MessageTaskDispatcher(
    const MessageTaskDispatcher &value
)
    : queue(value.queue), taskResponse(value.taskResponse), thread(value.thread), running(value.running)
{
}

MessageTaskDispatcher::~MessageTaskDispatcher()
{
    stop();
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

bool MessageTaskDispatcher::sendControl(
    const std::string &cmd
)
{
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        return false;
    sockaddr_in destination {
        .sin_family = AF_INET,
        .sin_port = htons(CONTROL_PORT)
    };
    destination.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    size_t sz = cmd.size();
    ssize_t ssz = sendto(sock, cmd.c_str(), sz, 0, (const sockaddr *) &destination, sizeof(destination));
    close(sock);
    return (ssz == sz);
}

bool MessageTaskDispatcher::start()
{
    if (running)
        return true;
    running = true;
    thread = new std::thread(std::bind(&MessageTaskDispatcher::runner, this));
    thread->detach();
    return running;
}

void MessageTaskDispatcher::stop()
{
    if (!running)
        return;
    // wake-up select()
    sendControl("exit");

    // wait until thread finish
    std::mutex m;
    std::unique_lock<std::mutex> lock(m);
    while (running && loopExit.wait_for(lock, std::chrono::seconds(1)) == std::cv_status::timeout) {
        // try wake-up select() if UDP packet is missed
        sendControl("exit");
    }
    // free up resources
    delete thread;
}

bool MessageTaskDispatcher::createSockets()
{
    auto *tsControl = new TaskSocket (INADDR_LOOPBACK, CONTROL_PORT, [](
        MessageTaskDispatcher *dispatcher,
        const char *buffer,
        size_t size
    ) {
        if (strncmp(buffer, "exit", size) == 0) {
            dispatcher->running = false;
            return -1;
        }
        return 0;
    });
    sockets.push_back(tsControl);
    return true;
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

void MessageTaskDispatcher::closeSockets() {
    // close all sockets
    for (auto s : sockets) {
        delete s;
    }
    // clear container
    sockets.clear();
}

int MessageTaskDispatcher::runner()
{
    if (!createSockets() || !openSockets()) {
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

    char buffer[300];

    while (running) {
        fd_set working_set;
        // Copy the master fd_set over to the working fd_set
        memcpy(&working_set, &master_set, sizeof(master_set));
        // Initialize the timeval struct
        timeout = { .tv_sec = DEF_TIMEOUT_SEC, .tv_usec = 0 };
        int rc = select(maxFD1, &working_set, nullptr, nullptr, &timeout);
        if (rc < 0) { // select error
            break;
        }
        if (rc == 0) { // select() timed out.
            continue;
        }
        for (auto s : sockets) {
            if (FD_ISSET(s->sock, &working_set)) {
                ssize_t sz = recv(s->sock, buffer, sizeof(buffer), 0);
                if (sz > 0) {
                    int r = s->cb(this, buffer, sz);
                    if (r < 0)
                        break;
                }
            }
        }
    }

    closeSockets();
    running = false;
    loopExit.notify_all();
    return CODE_OK;
}
