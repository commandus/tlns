#include <WinSock2.h>
#include <iostream>
#include <condition_variable>
#include "lorawan/task/task-timer-socket.h"

class Counter {
public:
    int count;
    std::condition_variable cv;
    Counter()
        : count(0)
    {

    }
};

VOID CALLBACK onTimer(
    PVOID lpParam,
    BOOLEAN timedOut
){
    Counter *c = (Counter *)lpParam;
    c->count++;
    std::cout << "Timer count " << c->count << std::endl;
    c->cv.notify_all();
}

static void testAPI() {
    HANDLE hTimer = nullptr;
    HANDLE hTimerQueue;
    Counter counter;
    std::mutex cv_m;
    std::unique_lock<std::mutex> lk(cv_m);

    // Create the timer queue.
    hTimerQueue = CreateTimerQueue();

    for(int i = 1; i < 10; i++) {
        CreateTimerQueueTimer(&hTimer, hTimerQueue, (WAITORTIMERCALLBACK) onTimer,
                              &counter, 1000, 0, WT_EXECUTEINTIMERTHREAD | WT_EXECUTEONLYONCE);
        counter.cv.wait(lk, [&counter, i] { return counter.count == i; });
        DeleteTimerQueueTimer(hTimerQueue, hTimer, nullptr);
    }

    DeleteTimerQueue(hTimerQueue);
    std::cout << "Success" << std::endl;
}

static void testSelect() {
    fd_set masterReadSocketSet;
    struct timeval timeout {};

    TaskTimerSocket taskTimerSocket;
    SOCKET ts = taskTimerSocket.openSocket();

    TASK_TIME t = std::chrono::system_clock::now() + std::chrono::seconds(5);
    taskTimerSocket.setStartupTime(t);

    FD_ZERO(&masterReadSocketSet);
    FD_SET(ts, &masterReadSocketSet);
    SOCKET maxFD1 = ts + 1;

    bool running = true;
    uint64_t cnt = 0;
    while (running) {
        fd_set workingSocketSet;
        memcpy(&workingSocketSet, &masterReadSocketSet, sizeof(masterReadSocketSet));
        // Initialize the timeval struct
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        int rc = select((int) maxFD1, &workingSocketSet, nullptr, nullptr, &timeout);
        if (rc < 0)     // select error
            break;
        if (rc == 0) {     // select time out
            std::cerr << "Time out" << std::endl;
            break;
        }
        if (FD_ISSET(ts, &workingSocketSet)) {
            int sz = recv(ts, (char *) &cnt, (int) sizeof(cnt), 0);
            std::cout << cnt << " size " << sz << std::endl;
            if (sz < 0)
                std::cerr << "Error " << GetLastError() << std::endl;

            TASK_TIME t = std::chrono::system_clock::now() + std::chrono::seconds(1);
            taskTimerSocket.setStartupTime(t);
        }
        running = cnt < 5;
    }

    taskTimerSocket.closeSocket();
}

int main(
    int argc,
    char *argv[]
)
{
    WSADATA wsaData;
    int r = WSAStartup(MAKEWORD(2, 2), &wsaData);

    // testAPI();
    testSelect();

    WSACleanup();

    return r;
}
