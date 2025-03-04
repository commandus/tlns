#ifndef TLNS_RUN_DOWNLINK_H
#define TLNS_RUN_DOWNLINK_H

#include <mutex>
#include "lorawan/task/message-task-dispatcher.h"

enum DOWNLINK_RUNNER_STATE {
    DLRS_STOPPED,
    DLRS_START,
    DLRS_RUN,
    DLRS_STOP
};

class RunDownlink {
private:
    std::mutex mutexState;
    std::condition_variable cvState;
    void runner();
public:
    MessageTaskDispatcher *dispatcher;
    DOWNLINK_RUNNER_STATE state;

    RunDownlink(MessageTaskDispatcher *dispatcher);
    ~RunDownlink();

    // override this method
    virtual void run() = 0;

    void start();
    void stop();
};

#endif
