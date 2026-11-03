#ifndef TASK_PLATFORM_H
#define TASK_PLATFORM_H
/*
 * Platform specific definitions
 */

#include <chrono>

typedef std::chrono::time_point<std::chrono::system_clock> TASK_TIME;

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
typedef struct in_addr in_addr_t;
#define sleep(x) Sleep(x)
// #define close(x) _close(x)
#endif

#endif
