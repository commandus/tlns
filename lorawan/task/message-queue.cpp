#include "message-queue.h"
#include "message-task-dispatcher.h"

#define DEF_MESSAGE_QUEUE_PRE_ALLOC 16
#define DEF_MESSAGE_QUEUE_MAX       128

MessageQueue::MessageQueue()
    : messageLimit(DEF_MESSAGE_QUEUE_MAX)
{
    items.reserve(DEF_MESSAGE_QUEUE_PRE_ALLOC);
}

void MessageQueue::setSize(
    size_t queuePreAlloc,
    size_t queueMax
)
{
    messageLimit = queueMax;
    items.reserve(queuePreAlloc);
}

MessageQueue::~MessageQueue() = default;

void MessageQueue::step()
{

}

void MessageQueue::setDispatcher(
    MessageTaskDispatcher *aDispatcher
) {
    dispatcher = aDispatcher;
}
