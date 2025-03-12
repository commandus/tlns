#include <thread>
#include "lorawan/downlink/run-downlink.h"

RunDownlink::RunDownlink(MessageTaskDispatcher *aDispatcher)
    : dispatcher(aDispatcher), state(TASK_STOPPED)
{

}

RunDownlink::~RunDownlink()
{
    stop();
}

void RunDownlink::runner()
{
    state = TASK_RUN;
    run();
    // here state == TASK_STOP, set to TASK_STOPPED
    std::unique_lock<std::mutex> lck(mutexState);
    state = TASK_STOPPED;
    cvState.notify_all();
}

void RunDownlink::start()
{
    if (state != TASK_STOPPED)
        return;
    state = TASK_START;
    std::thread t(&RunDownlink::runner, this);
    t.detach();
}

void RunDownlink::stop()
{
    state = TASK_STOP;
    // wait until thread finished
    std::unique_lock<std::mutex> lock(mutexState);
    while(state != TASK_STOPPED)
        cvState.wait(lock);
}
