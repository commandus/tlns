#include "lorawan/task/task-timer-socket.h"
#if defined(_MSC_VER) || defined(__MINGW32__)
#define close(x) closesocket(x)
#else
#include <unistd.h>
#include <sys/timerfd.h>
#endif

#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-error.h"

TaskTimerSocket::TaskTimerSocket()
    : TaskSocket(SA_TIMER), currentTime{}
{
}

SOCKET TaskTimerSocket::openSocket()
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    /* @see https://blog.grijjy.com/2017/04/20/cross-platform-timer-queues-for-windows-and-linux/
     * TimerQueueHandle h = CreateTimerQueue();
     * DeleteTimerQueueEx(h, INVALID_HANDLE_VALUE);
     * if (CreateTimerQueueTimer(h, TimerQueueHandle, @WaitOrTimerCallback, MyObject, 0, Interval, 0))
     * {}
     */
    sock = INVALID_SOCKET;
#else
    sock = timerfd_create(CLOCK_REALTIME, 0);
#endif
    if (sock <= 0)
        lastError = ERR_CODE_SOCKET_CREATE;
    return sock;
}

void TaskTimerSocket::closeSocket()
{
    if (sock > 0) {
        close(sock);
        sock = -1;
    }
}

// virtual int onData(const char *buffer, size_t size) = 0;
TaskTimerSocket::~TaskTimerSocket()
{
    closeSocket();
}

/**
 * Set expiration time for timer
 * @param time time to set
 * @return true if success
 */
bool TaskTimerSocket::setStartupTime(
    TASK_TIME time
)
{
    // getUplink seconds
    auto s = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch());
    // getUplink nanoseconds: extract seconds from the time
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time.time_since_epoch());
    ns -= s;
    // set timer structure
    struct itimerspec t = {
        .it_interval = { .tv_sec = 0, .tv_nsec = 0},
        .it_value = { .tv_sec = s.count(), .tv_nsec = ns.count()}
    };
    // return false if failed
#if defined(_MSC_VER) || defined(__MINGW32__)
    return false;
#else
    return timerfd_settime(sock, TFD_TIMER_ABSTIME, &t, nullptr) == 0;
#endif
}
