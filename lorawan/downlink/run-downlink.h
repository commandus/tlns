#ifndef TLNS_RUN_DOWNLINK_H
#define TLNS_RUN_DOWNLINK_H

#include <mutex>
#include "lorawan/task/task-state.h"
#include "lorawan/task/message-task-dispatcher.h"

class RunDownlink {
private:
    std::mutex mutexState;
    std::condition_variable cvState;
    void runner();
public:
    MessageTaskDispatcher *dispatcher;
    TASK_STATE state;

    RunDownlink(MessageTaskDispatcher *dispatcher);
    ~RunDownlink();

    // override this method
    virtual void run() = 0;

    void start();
    void stop();
};

#endif
