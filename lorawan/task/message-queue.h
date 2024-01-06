#include <vector>

#include "message-queue-item.h"

class MessageQueue {
protected:
    size_t messageLimit;
public:
    std::vector <MessageQueueItem> items;
    MessageQueue();
    void setSize(size_t queuePreAlloc, size_t queueMax);
    virtual ~MessageQueue();
    void step();
};
