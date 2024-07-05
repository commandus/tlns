#include "lorawan/task/task-timer-socket.h"
#include <unistd.h>
#include <sys/timerfd.h>
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-error.h"

TaskTimerSocket::TaskTimerSocket()
    : TaskSocket(SA_TIMER), currentTime{}
{
}

SOCKET TaskTimerSocket::openSocket()
{
    sock = timerfd_create(CLOCK_REALTIME, 0);
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

void TaskTimerSocket::setStartupTime(
    TASK_TIME time
)
{
    auto s = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch());
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time.time_since_epoch());
    struct itimerspec t = {
        .it_interval = { .tv_sec = 0, .tv_nsec = 0},
        .it_value = { .tv_sec = s.count(), .tv_nsec = ns.count()}
    };
    timerfd_settime(sock, 0, &t, nullptr);
}
