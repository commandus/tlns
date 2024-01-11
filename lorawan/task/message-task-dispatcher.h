#ifndef MESSAGE_TASK_DISPATCHER_H
#define MESSAGE_TASK_DISPATCHER_H

#include <thread>
#include <condition_variable>
#include "message-queue.h"
#include "task-response.h"

class MessageTaskDispatcher {
protected:
    MessageQueue *queue;
    TaskResponse *taskResponse;
    std::thread *thread;
    mutable std::condition_variable loopExit;
public:
    bool running;

    void runner();

    MessageTaskDispatcher();
    MessageTaskDispatcher(const MessageTaskDispatcher &value);
    virtual ~MessageTaskDispatcher();

    void setQueue(MessageQueue *queue);
    void response(MessageQueueItem *item);
    void setResponse(TaskResponse *receiver);

    bool start();
    void stop();
};

#endif
