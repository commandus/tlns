#include <iostream>
#include "lorawan/task/task-timer-socket.h"
#if defined(_MSC_VER) || defined(__MINGW32__)
#define DEF_UDP_TIMER_PORT  54321
#else
#include <unistd.h>
#include <sys/timerfd.h>
#endif

#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-error.h"

TaskTimerSocket::TaskTimerSocket()
    : TaskSocket(SA_TIMER)
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    hTimerQueue = CreateTimerQueue();
    count = 0;
#endif
}

SOCKET TaskTimerSocket::openSocket()
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        lastError = ERR_CODE_SOCKET_CREATE;
        return INVALID_SOCKET;
    }
    // Allow socket descriptor to be reusable
    int on = 1;
    int rc = setsockopt(sock, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
    if (rc < 0) {
        closesocket(sock);
        sock = INVALID_SOCKET;
        lastError = ERR_CODE_SOCKET_OPEN;
        return INVALID_SOCKET;
    }
    // Set socket to be nonblocking
    u_long onw = 1;
    rc = ioctlsocket(sock, FIONBIO, &onw);
    // Bind the socket
    struct sockaddr_in saddr {};
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    saddr.sin_port = DEF_UDP_TIMER_PORT;
    rc = bind(sock, (struct sockaddr *) &saddr, sizeof(saddr));
    if (rc < 0) {
        std::cerr << GetLastError() << std::endl;
        closesocket(sock);
        sock = INVALID_SOCKET;
        lastError = ERR_CODE_SOCKET_BIND;
        return INVALID_SOCKET;
    }
    lastError = CODE_OK;
    return sock;
#else
    sock = timerfd_create(CLOCK_REALTIME, 0);
#endif
    if (sock <= 0)
        lastError = ERR_CODE_SOCKET_CREATE;
    return sock;
}

void TaskTimerSocket::closeSocket()
{
    if (sock != INVALID_SOCKET) {
#if defined(_MSC_VER) || defined(__MINGW32__)
        closesocket(sock);
#else
        close(sock);
#endif
        sock = INVALID_SOCKET;
    }
}

// virtual int onData(const char *buffer, size_t size) = 0;
TaskTimerSocket::~TaskTimerSocket()
{
    closeSocket();
#if defined(_MSC_VER) || defined(__MINGW32__)
    DeleteTimerQueue(hTimerQueue);
#endif
}

#if defined(_MSC_VER) || defined(__MINGW32__)
void TaskTimerSocket::onWindowsTimer() {
    count++;

    struct sockaddr_in saddr {};
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    saddr.sin_port = DEF_UDP_TIMER_PORT;

    int sz = sendto(sock, (const char*) &count, (int) sizeof(uint64_t), 0, (const sockaddr*) &saddr, sizeof(saddr));
    std::cerr << "Sent Time " << count << " size " << sz << std::endl;
}

VOID CALLBACK onTimerCb(
    PVOID lpParam,
    BOOLEAN timedOut
) {
    TaskTimerSocket *s = (TaskTimerSocket *) lpParam;
    DeleteTimerQueueTimer(s->hTimerQueue, s->hTimer, nullptr);
    std::cerr << "Time " << std::endl;
    s->onWindowsTimer();
}
#endif

/**
 * Set expiration time for timer
 * @param tim time to set
 * @return true if success
 */
int TaskTimerSocket::setStartupTime(
    TASK_TIME tim
)
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            (tim - std::chrono::system_clock::now())
    );
    BOOL r = CreateTimerQueueTimer(&hTimer, hTimerQueue, (WAITORTIMERCALLBACK) onTimerCb,
        this, (DWORD) ms.count(), 0, WT_EXECUTEINTIMERTHREAD | WT_EXECUTEONLYONCE);
    std::cout << "Set startup time " << (r ? "success" : "failed") << std::endl;
    return r ? CODE_OK : ERR_CODE_PARAM_INVALID;
#else
    // getUplink seconds
    auto s = std::chrono::duration_cast<std::chrono::seconds>(tim.time_since_epoch());
    // getUplink nanoseconds: extract seconds from the time
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(tim.time_since_epoch());
    ns -= s;
    // set timer structure
    struct itimerspec t = {
        .it_interval = { .tv_sec = 0, .tv_nsec = 0 },
        .it_value = { .tv_sec = s.count(), .tv_nsec = static_cast<long>(ns.count()) }
    };
    return timerfd_settime(sock, TFD_TIMER_ABSTIME, &t, nullptr);
#endif
}
