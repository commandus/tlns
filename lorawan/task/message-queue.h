#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <vector>

#include "message-queue-item.h"

class MessageTaskDispatcher;

class MessageQueue {
protected:
    size_t messageLimit;
    MessageTaskDispatcher *dispatcher;
public:
    std::vector <MessageQueueItem> items;
    MessageQueue();
    void setSize(size_t queuePreAlloc, size_t queueMax);
    virtual ~MessageQueue();
    void step();
    void setDispatcher(
        MessageTaskDispatcher *aDispatcher
    );
};

#endif
