#include <functional>
#include <csignal>
#include "message-task-dispatcher.h"

MessageTaskDispatcher::MessageTaskDispatcher()
    : queue(nullptr), taskResponse(nullptr), thread(nullptr), running(false)
{

}

MessageTaskDispatcher::MessageTaskDispatcher(
    const MessageTaskDispatcher &value
)
    : queue(value.queue), taskResponse(value.taskResponse), thread(value.thread), running(value.running)
{
}

MessageTaskDispatcher::~MessageTaskDispatcher()
{
    stop();
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

void MessageTaskDispatcher::setResponse(
    TaskResponse *value
)
{
    taskResponse = value;
}

bool MessageTaskDispatcher::start()
{
    if (running)
        return true;
    running = true;
    thread = new std::thread(std::bind(&MessageTaskDispatcher::runner, this));
    thread->detach();
    return true;
}

void MessageTaskDispatcher::stop()
{
    if (!running)
        return;
    running = false;
    std::mutex m;
    std::unique_lock<std::mutex> lock(m);
    loopExit.wait(lock);
    delete thread;

}

void MessageTaskDispatcher::runner()
{
    while (running) {
        // receive
        sleep(1);
    }
    loopExit.notify_all();
}

