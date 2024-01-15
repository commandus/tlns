#ifndef MESSAGE_TASK_DISPATCHER_H
#define MESSAGE_TASK_DISPATCHER_H

#include <thread>
#include <condition_variable>

#include "lorawan/task/message-queue.h"
#include "lorawan/task/task-response.h"
#include "lorawan/helper/ip-address.h"

class MessageTaskDispatcher {
private:
    SOCKET fdControl;
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
    virtual ~MessageTaskDispatcher();

    void setQueue(MessageQueue *queue);
    void response(MessageQueueItem *item);
    void setResponse(TaskResponse *receiver);
    bool sendControl(const std::string &cmd);

    bool start();
    void stop();
};

#endif
