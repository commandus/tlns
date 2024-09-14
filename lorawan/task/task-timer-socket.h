#ifndef TASK_TIMER_SOCKET_H_
#define TASK_TIMER_SOCKET_H_ 1

#include "lorawan/task/task-platform.h"
#include "lorawan/task/task-socket.h"

/**
 */
class TaskTimerSocket : public TaskSocket {
public:
    TASK_TIME currentTime;
    /**
     * Task timer socket
     */
    TaskTimerSocket();
    /**
     * Open timer socket
     * @return socket number, -1 if fails
     */
    SOCKET openSocket() override;
    void closeSocket() override;
    virtual ~TaskTimerSocket();
    TASK_TIME getStartupTime();
    bool setStartupTime(TASK_TIME time);
};

#endif
