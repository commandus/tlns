#include <functional>
#include <csignal>
#include <iostream>

#include "task-response-threaded.h"

void TaskResponseThreaded::onReceive(
    MessageQueueItem *value
) {
    TaskResponse::onReceive(value);
}

void TaskResponseThreaded::onResponse(
    MessageQueueItem* value
)
{
    TaskResponse::onResponse(value);
}

TaskResponseThreaded::TaskResponseThreaded()
    : TaskResponse()
{
    start();
}

bool TaskResponseThreaded::start()
{
    if (!TaskResponse::start())
        return false;
    thread = new std::thread(std::bind(&TaskResponseThreaded::run, this));
    thread->detach();
    return true;
}

void TaskResponseThreaded::stop()
{
    if (!running)
        return;
    TaskResponse::stop();
    std::mutex m;
    std::unique_lock<std::mutex> lock(m);
    loopExit.wait(lock);
    delete thread;
    std::cerr << "stopped" << std::endl;
}

void TaskResponseThreaded::run()
{
    while (running) {
        // receive
        MessageQueueItem* receivedItem = new MessageQueueItem;
        std::cerr << "Received " << receivedItem->toString() << std::endl;

        sleep(1);
    }
    loopExit.notify_all();
    std::cerr << "run out" << std::endl;
}

TaskResponseThreaded::~TaskResponseThreaded()
{
    stop();
}
