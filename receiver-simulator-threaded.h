#ifndef RECEIVER_SIMULATOR_THREADED_H
#define RECEIVER_SIMULATOR_THREADED_H

#include <condition_variable>
#include "thread"
#include "lorawan/task/task-response.h"

class ReceiverSimulatorThreaded : public TaskResponse {
private:
    std::thread *thread;
    mutable std::condition_variable cv;
protected:
    bool running;
    void run();
public:
    ReceiverSimulatorThreaded();
    virtual ~ReceiverSimulatorThreaded();
    void onReceive(MessageQueueItem* value) override;
    void onResponse(MessageQueueItem* value) override;

    void start();
    void stop();
};

#endif
