#ifndef TASK_RESPONSE_H
#define TASK_RESPONSE_H

#include "vector"

#include "lorawan/task/message-queue-item.h"

/**
 * Interface class
 */
class TaskResponse {
public:
    bool running;
    std::vector<MessageQueueItem *> queue;

    TaskResponse();
    TaskResponse(const TaskResponse &value);
    virtual ~TaskResponse();
    virtual void onReceive(MessageQueueItem* value);
    virtual void onResponse(MessageQueueItem* value);
    virtual bool start();
    virtual void stop();

};

#endif
