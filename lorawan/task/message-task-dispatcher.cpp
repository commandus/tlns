#include "message-task-dispatcher.h"

MessageTaskDispatcher::MessageTaskDispatcher()
    : queue(nullptr)
{

}

MessageTaskDispatcher::MessageTaskDispatcher(
    const MessageTaskDispatcher &value
)
    : queue(value.queue)
{
}

void MessageTaskDispatcher::setQueue(
    MessageQueue *aQueue
)
{
    queue = aQueue;
}

void MessageTaskDispatcher::response(
    MessageQueueItem *item
)
{

}
