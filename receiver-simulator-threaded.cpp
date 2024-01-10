#include <functional>
#include <csignal>
#include <iostream>

#include "receiver-simulator-threaded.h"

void ReceiverSimulatorThreaded::onReceive(
    MessageQueueItem *value
) {
    TaskResponse::onReceive(value);
}

void ReceiverSimulatorThreaded::onResponse(
    MessageQueueItem* value
)
{
    TaskResponse::onResponse(value);
}

ReceiverSimulatorThreaded::ReceiverSimulatorThreaded()
    : TaskResponse()
{
    start();
}

void ReceiverSimulatorThreaded::stop()
{
    if (!running)
        return;
    running = false;
    std::mutex cv_m;
    std::unique_lock<std::mutex> lock(cv_m);
    std::cerr << "stopped" << std::endl;
    delete thread;
    thread = nullptr;
    cv.wait(lock);
}

void ReceiverSimulatorThreaded::run()
{
    running = true;
    while (running) {
        std::cerr << "run.." << std::endl;
        sleep(1);
    }
    std::cerr << "run out" << std::endl;
    cv.notify_all();
}

ReceiverSimulatorThreaded::~ReceiverSimulatorThreaded()
{
    stop();
}

void ReceiverSimulatorThreaded::start()
{
    thread = new std::thread(std::bind(&ReceiverSimulatorThreaded::run, this));
    thread->detach();
}
