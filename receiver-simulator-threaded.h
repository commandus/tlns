#ifndef RECEIVER_SIMULATOR_THREADED_H
#define RECEIVER_SIMULATOR_THREADED_H

#include "thread"
#include "lorawan/task/task-response.h"

class ReceiverSimulatorThreaded : public TaskResponse {
public:
    ReceiverSimulatorThreaded();
    virtual ~ReceiverSimulatorThreaded();
    void onReceive(MessageQueueItem* value) override;
    void onResponse(MessageQueueItem* value) override;
};

#endif
