#ifndef TASK_UDP_SOCKET_H_
#define TASK_UDP_SOCKET_H_ 1

#include <map>
#include "lorawan/lorawan-types.h"
#include "lorawan/task/task-platform.h"

class TimeAddr {
public:
    TASK_TIME startTime;
    DEVADDR addr;
    TimeAddr();
    TimeAddr(const TimeAddr &value);
    bool operator==(const TimeAddr &rhs) const;
    bool operator>(const TimeAddr &rhs) const;
    bool operator<(const TimeAddr &rhs) const;
    bool operator!=(const TimeAddr &rhs) const;

    std::string toString();
};

class TimeAddrSet {
public:
    std::map<TASK_TIME, DEVADDR> timeAddr;
    std::map<DEVADDR, TASK_TIME> addrTime;

    TimeAddrSet();

    void push(
        DEVADDR addr,
        TASK_TIME time
    );
    bool peek(TimeAddr &retVal) const;

    /**
     * explicitly pop latest address & time pair
     * @param retVal can be NULL
     * @return true if success
     */
    bool pop(TimeAddr *retVal);

    /**
     * pop latest address & time pair if time is greater than specified time
     * @param retVal returning value
     * @param time
     * @return true if
     */
    bool pop(TimeAddr &retVal, TASK_TIME since);

    /**
     * Check is address in the queue
     * @param addr device address
     * @return
     */
    bool has(DEVADDR addr) const;

    /**
     * Return received time if address in the queue. Return 0 from epoch if it is not.
     * @param addr address to check
     * @return received time
     */
    TASK_TIME taskTime(DEVADDR addr) const;
    /**
     * Clear
     * @param expiration if NULL, clear all
     */
    void clear(TASK_TIME *expiration);

    /**
     * Return queue size
     * @return queue size
     */
    size_t size() const;

    /**
     * Calculate time in microseconds after "since" parameter when response must be send.
     * There is WAIT_TIME_FOR_ALL_GATEWAYS_IN_MICROSECONDS time server wait until messages
     * received from all gateways before response must be send.
     * @param since current time
     * @return  if no messages to respond, return DEF_TIME_FOR_ALL_GATEWAYS_IN_MICROSECONDS.
     * Otherwise it is time of the oldest message plus required delay.
     */
    long waitTimeForAllGatewaysInMicroseconds(TASK_TIME since) const;
};

#endif
