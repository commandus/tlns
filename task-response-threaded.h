#ifndef TASK_RESPONSE_THREADED_H
#define TASK_RESPONSE_THREADED_H

#include <condition_variable>
#include "thread"
#include "lorawan/task/task-response.h"

class TaskResponseThreaded : public TaskResponse {
private:
    std::thread *thread;
    mutable std::condition_variable loopExit;
protected:
    void run();
public:
    TaskResponseThreaded();
    virtual ~TaskResponseThreaded();
    void onReceive(MessageQueueItem* value) override;
    void onResponse(MessageQueueItem* value) override;

    bool start() override;
    void stop() override;
};

#endif
