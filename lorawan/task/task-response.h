#ifndef TASK_RESPONSE_H
#define TASK_RESPONSE_H

#include "vector"

#include "lorawan/task/message-queue-item.h"

/**
 * Interface class
 */
class TaskResponse {
protected:
    std::vector<MessageQueueItem *> queue;
public:
    TaskResponse();
    TaskResponse(const TaskResponse &value);
    virtual ~TaskResponse();
    virtual void onReceive(MessageQueueItem* value);
    virtual void onResponse(MessageQueueItem* value);
};

#endif
