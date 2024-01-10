#ifndef MESSAGE_TASK_DISPATCHER_H
#define MESSAGE_TASK_DISPATCHER_H

#include "message-queue.h"
#include "task-response.h"

class MessageTaskDispatcher {
protected:
    MessageQueue *queue;
    TaskResponse *receiver;

public:
    MessageTaskDispatcher();
    MessageTaskDispatcher(const MessageTaskDispatcher &value);
    void setQueue(MessageQueue *queue);
    void response(MessageQueueItem *item);
    void setReceiver(TaskResponse *receiver);
};

#endif
