#ifndef TASK_DISPATCHER_H
#define TASK_DISPATCHER_H

#include "message-queue.h"

class MessageTaskDispatcher {
protected:
    MessageQueue *queue;
public:
    MessageTaskDispatcher();
    MessageTaskDispatcher(const MessageTaskDispatcher &value);
    void setQueue(MessageQueue *queue);
    void response(MessageQueueItem *item);
};

#endif
