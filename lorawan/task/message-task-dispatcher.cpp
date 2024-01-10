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
    queue->setDispatcher(this);
}

void MessageTaskDispatcher::response(
    MessageQueueItem *item
)
{

}

void MessageTaskDispatcher::setReceiver(
    TaskResponse *aReceiver
)
{
    receiver = aReceiver;
}