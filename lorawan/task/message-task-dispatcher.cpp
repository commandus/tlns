#include "message-task-dispatcher.h"

#include <functional>
#include <csignal>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <iostream>

#include "lorawan/lorawan-error.h"

MessageTaskDispatcher::MessageTaskDispatcher()
    : fdControl(0), queue(nullptr), taskResponse(nullptr), thread(nullptr), running(false)
{

}

MessageTaskDispatcher::MessageTaskDispatcher(
    const MessageTaskDispatcher &value
)
    : fdControl(0), queue(value.queue), taskResponse(value.taskResponse), thread(value.thread), running(value.running)
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

#define CONTROL_PORT 4242

bool MessageTaskDispatcher::sendControl(
    const std::string &cmd
) const
{
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        return false;
    sockaddr_in destination;
    destination.sin_family = AF_INET;
    destination.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    destination.sin_port = htons(CONTROL_PORT);
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
    running = false;

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

int MessageTaskDispatcher::runner()
{
    fdControl = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fdControl < 0) {
        running = false;
        return ERR_CODE_SOCKET_CREATE;
    }
    struct timeval timeout {};

    // Allow socket descriptor to be reuseable
    int on = 1;
    int rc = setsockopt(fdControl, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
    if (rc < 0) {
        close(fdControl);
        running = false;
        return ERR_CODE_SOCKET_OPEN;
    }

    // Set socket to be nonblocking
    rc = ioctl(fdControl, FIONBIO, (char *)&on);
    if (rc < 0) {
        close(fdControl);
        running = false;
        return ERR_CODE_SOCKET_OPEN;
    }

    // Bind the socket
    struct sockaddr_in addr {};
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // INADDR_ANY;
    addr.sin_port = htons(CONTROL_PORT);
    rc = bind(fdControl, (struct sockaddr *) &addr, sizeof(addr));
    if (rc < 0) {
        close(fdControl);
        running = false;
        return ERR_CODE_SOCKET_BIND;
    }

    // Initialize the master fd_set
    fd_set master_set;
    // , working_set;
    FD_ZERO(&master_set);
    SOCKET minFD = fdControl;
    SOCKET maxFD = fdControl;
    FD_SET(fdControl, &master_set);
    // Initialize the timeval struct
    timeout.tv_sec  = 3 * 60;
    timeout.tv_usec = 0;

    char buffer[10];

    while (running) {
        fd_set working_set;
        // Copy the master fd_set over to the working fd_set
        memcpy(&working_set, &master_set, sizeof(master_set));
        rc = select(maxFD + 1, &working_set, nullptr, nullptr, &timeout);
        if (rc < 0) {
            // select error
            break;
        }
        if (rc == 0) {
            // select() timed out.
            std::cerr << "timed out" << std::endl;
            continue;
        }
        for (int i = minFD; i <= maxFD; i++) {
            if (FD_ISSET(i, &working_set)) {
                if (i == fdControl) {
                    ssize_t sz = recv(i, buffer, sizeof(buffer), 0);
                    if (sz > 0) {
                        std::string s(buffer, sz);
                        if (s == "exit") {
                            running = false;
                            break;
                        }
                    }
                }
            }
        }
        sleep(1);
    }
    close(fdControl);
    fdControl = -1;
    loopExit.notify_all();
}
