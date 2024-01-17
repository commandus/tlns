#ifndef MESSAGE_TASK_DISPATCHER_H
#define MESSAGE_TASK_DISPATCHER_H

#include <thread>
#include <condition_variable>

#include "lorawan/task/message-queue.h"
#include "lorawan/task/task-response.h"
#include "lorawan/helper/ip-address.h"

class MessageTaskDispatcher;

typedef int(*TaskProc)(
    MessageTaskDispatcher *env,
    const char *buffer,
    size_t size
);

class TaskSocket {
public:
    SOCKET sock;
    in_addr_t addr;
    uint16_t port;
    int lastError;
    TaskProc cb;
    /**
     * @param addr ""- any interface, "localhost"- localhost otherwise- address
     * @param port port number
     */
    TaskSocket(in_addr_t intfType, uint16_t port, TaskProc cb);
    /**
     * Open UDP socket for listen
     * @return -1 if fail
     */
    SOCKET openUDPSocket();
    void closeSocket();
    // virtual int onData(const char *buffer, size_t size) = 0;
    virtual ~TaskSocket();
};

class MessageTaskDispatcher {
protected:
    MessageQueue *queue;
    TaskResponse *taskResponse;
    std::thread *thread;
    mutable std::condition_variable loopExit;

    bool openSockets();
    /**
     * close all sockets
     */
    void closeSockets();
    void clearSockets();
public:
    TaskSocket* controlSocket;
    std::vector<TaskSocket*> sockets;
    bool running;

    int runner();

    MessageTaskDispatcher();
    MessageTaskDispatcher(const MessageTaskDispatcher &value);
    void setPorts(uint16_t control);
    virtual ~MessageTaskDispatcher();

    void setQueue(MessageQueue *queue);
    void response(MessageQueueItem *item);
    void setResponse(TaskResponse *receiver);
    bool sendControl(const std::string &cmd);

    bool start();
    void stop();
};

/**
 * Create control socket
 * @param dispatcher owner
 * @param addr socket address
 * @param port port number
 * @return socket, -1 if fail
 */
SOCKET addControlSocket(
    MessageTaskDispatcher *dispatcher,
    in_addr_t addr,
    uint16_t port
);

#endif
