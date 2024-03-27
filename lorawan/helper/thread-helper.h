#ifndef THREAD_HELPER_H_
#define THREAD_HELPER_H_ 1

#include <thread>

void setThreadName(
    std::thread* thread,
    const char* threadName
);

#endif
