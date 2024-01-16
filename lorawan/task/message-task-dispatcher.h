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
    std::string addr;
    uint16_t port;
    int lastError;
    TaskProc cb;
    /**
     * @param addr ""- any interface, "localhost"- localhost otherwise- address
     * @param port port number
     */
    TaskSocket(const std::string &addr, uint16_t port, TaskProc cb);
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
private:
    std::vector<TaskSocket*> sockets;
protected:
    MessageQueue *queue;
    TaskResponse *taskResponse;
    std::thread *thread;
    mutable std::condition_variable loopExit;
public:
    bool running;

    int runner();

    MessageTaskDispatcher();
    MessageTaskDispatcher(const MessageTaskDispatcher &value);
    void setPorts(uint16_t control);
    virtual ~MessageTaskDispatcher();

    void setQueue(MessageQueue *queue);
    void response(MessageQueueItem *item);
    void setResponse(TaskResponse *receiver);
    bool sendControl(const std::string &cmd) const;

    bool start();
    void stop();
};

#endif
