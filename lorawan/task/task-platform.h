#ifndef TASK_PLATFORM_H
#define TASK_PLATFORM_H
/*
 * Platform specific definitions
 */

#include <chrono>

typedef std::chrono::time_point<std::chrono::system_clock> TASK_TIME;

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
typedef struct in_addr in_addr_t;
#define sleep(x) Sleep(x)
#define close(x) closesocket(x)
#endif

#endif
