#include <thread>
#include "lorawan/downlink/run-downlink.h"

RunDownlink::RunDownlink(MessageTaskDispatcher *aDispatcher)
    : dispatcher(aDispatcher), state(DLRS_STOPPED)
{

}

RunDownlink::~RunDownlink()
{
    stop();
}

void RunDownlink::runner()
{
    state = DLRS_RUN;
    run();
    // here state == DLRS_STOP, set to DLRS_STOPPED
    std::unique_lock<std::mutex> lck(mutexState);
    state = DLRS_STOPPED;
    cvState.notify_all();
}

void RunDownlink::start()
{
    if (state != DLRS_STOPPED)
        return;
    state = DLRS_START;
    std::thread t(&RunDownlink::runner, this);
    t.detach();
}

void RunDownlink::stop()
{
    state = DLRS_STOP;
    // wait until thread finished
    std::unique_lock<std::mutex> lock(mutexState);
    while(state != DLRS_STOPPED)
        cvState.wait(lock);
}
