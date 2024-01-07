#ifndef MESSAGE_TASK_DISPATCHER_H
#define MESSAGE_TASK_DISPATCHER_H

#include "message-queue.h"

class MessageTaskDispatcher {
protected:
    MessageQueue *queue;
public:
    MessageTaskDispatcher();
    MessageTaskDispatcher(const MessageTaskDispatcher &value);
    void setReceiver();
    void setQueue(MessageQueue *queue);
    void response(MessageQueueItem *item);
};

#endif
