#ifndef TASK_TIMER_SOCKET_H_
#define TASK_TIMER_SOCKET_H_ 1

#include "lorawan/task/task-platform.h"
#include "lorawan/task/task-socket.h"

/**
 */
class TaskTimerSocket : public TaskSocket {
public:
#if defined(_MSC_VER) || defined(__MINGW32__)
    HANDLE hTimerQueue;
    HANDLE hTimer;
    uint64_t count;
    uint16_t nPort; ///< random assigned port (in network byte order)
    void onWindowsTimer();
#endif

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
    /**
     * set time
     * @param time time
     * @return 0- success
     */
    int setStartupTime(TASK_TIME time);
};

#endif
