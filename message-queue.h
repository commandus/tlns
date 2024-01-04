#include <vector>

#include "message-queue-item.h"

class MessageQueue {
public:
    std::vector <MessageQueueItem> items;

    explicit MessageQueue(size_t queuePreAlloc, size_t queueMax);
    void step();
};
