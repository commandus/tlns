#ifndef TASK_UDP_SOCKET_H_
#define TASK_UDP_SOCKET_H_ 1

#include <map>
#include "lorawan/lorawan-types.h"
#include "lorawan/task/task-platform.h"

class TimeAddr {
public:
    TASK_TIME startTime;
    DEVADDR addr;
    bool operator==(const TimeAddr &rhs) const;
    bool operator>(const TimeAddr &rhs) const;
    bool operator<(const TimeAddr &rhs) const;
    bool operator!=(const TimeAddr &rhs) const;
};

class TimeAddrSet {
public:
    std::map<TASK_TIME, DEVADDR> timeAddr;
    std::map<DEVADDR, TASK_TIME> addrTime;

    TimeAddrSet();

    void push(DEVADDR addr, TASK_TIME time);
    bool peek(TimeAddr &retVal) const;
    /**
     * pop latest address &time
     * @param retVal can be NULL
     * @return
     */
    bool pop(TimeAddr *retVal);
    bool has(DEVADDR addr) const;
    TASK_TIME taskTime(DEVADDR addr) const;
    /**
     * Clear
     * @param expiration if NULL, clear all
     */
    void clear(TASK_TIME *expiration);
    size_t size() const;
    long waitTimeMicroseconds(TASK_TIME since) const;
};

#endif
