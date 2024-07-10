#include <iostream>
#include "lorawan/task/task-time-addr.h"
#include "lorawan/lorawan-date.h"

// set for 1s
#define DEF_TIME_FOR_ALL_GATEWAYS_IN_MICROSECONDS   1000000
// set for 1/2s
#define WAIT_TIME_FOR_ALL_GATEWAYS_IN_MICROSECONDS   500000

TimeAddr::TimeAddr()
    : startTime {}, addr(0)
{

}

TimeAddr::TimeAddr(
    const TimeAddr &value
)
    : startTime(value.startTime), addr(value.addr)
{

}

bool TimeAddr::operator==(
    const TimeAddr &rhs
) const
{
    return startTime == rhs.startTime && addr == rhs.addr;
}

bool TimeAddr::operator>(
    const TimeAddr &rhs
) const
{
    return (startTime > rhs.startTime) || ((startTime == rhs.startTime) && (addr > rhs.addr));
}

bool TimeAddr::operator<(
    const TimeAddr &rhs
) const
{
    return (startTime < rhs.startTime) || ((startTime == rhs.startTime) && (addr < rhs.addr));
}

bool TimeAddr::operator!=(
    const TimeAddr &rhs
) const
{
    return (startTime != rhs.startTime) || (addr != rhs.addr);
}

std::string TimeAddr::toString() {
    return addr.toString() + '\t' + taskTime2string(startTime);
}

TimeAddrSet::TimeAddrSet()
{
}

void TimeAddrSet::push(
    DEVADDR addr,
    TASK_TIME time
)
{
    auto ta = timeAddr.begin();
    if (ta != timeAddr.end())
        return;
    timeAddr[time] = addr;
    addrTime[addr] = time;
}

bool TimeAddrSet::peek(
    TimeAddr &retVal
) const
{
    auto ta = timeAddr.begin();
    if (ta == timeAddr.end())
        return false;
    retVal.startTime = ta->first;
    retVal.addr = ta->second;
    return true;
}

/**
 * pop latest address &time
 * @param retVal can be NULL
 * @return
 */
bool TimeAddrSet::pop(
    TimeAddr *retVal
)
{
    auto ta = timeAddr.begin();
    bool b = ta != timeAddr.end();
    if (b) {
        if (retVal) {
            retVal->addr = ta->second;
            retVal->startTime = ta->first;
        }
        timeAddr.erase(ta);

        auto f = addrTime.find(ta->second);
        if (f == addrTime.end()) {
            addrTime.erase(f);
        }
    }
    return b;
}

bool TimeAddrSet::pop(
    TimeAddr &retVal,
    TASK_TIME time
)
{
    auto ta = timeAddr.begin();
    bool b = ta != timeAddr.end();
    if (b) {
        std::cout << "## Pop now " << taskTime2string(time) << " ";
        time -= std::chrono::microseconds(WAIT_TIME_FOR_ALL_GATEWAYS_IN_MICROSECONDS);
        std::cout << " - " << taskTime2string(time) << " " << taskTime2string(ta->first) << "\n";
        if (ta->first > time)
            return false;
        retVal.addr = ta->second;
        retVal.startTime = ta->first;
        timeAddr.erase(ta);

        auto f = addrTime.find(ta->second);
        if (f != addrTime.end()) {
            addrTime.erase(f);
        }
    }
    return b;
}

/**
 * Calculate time in microseconds after "since" parameter when response must be send.
 * There is WAIT_TIME_FOR_ALL_GATEWAYS_IN_MICROSECONDS time server wait until messages
 * received from all gateways before response must be send.
 * @param since current time
 * @return  if no messages to respond, return -1
 * Otherwise it is time of the oldest message plus required delay.
 */
long TimeAddrSet::waitTimeForAllGatewaysInMicroseconds(
    TASK_TIME since
) const
{
    auto ta = timeAddr.begin();
    if (ta == timeAddr.end())
        return -1;
    auto delta = std::chrono::duration_cast<std::chrono::microseconds>(ta->first
            + std::chrono::microseconds(WAIT_TIME_FOR_ALL_GATEWAYS_IN_MICROSECONDS) - since);
    auto r = delta.count();
    if (r < 0)
        return 0;
    return r;
}

bool TimeAddrSet::has(
    DEVADDR addr
) const
{
    auto a = addrTime.find(addr);
    return a != addrTime.end();
}

TASK_TIME TimeAddrSet::taskTime(
    DEVADDR addr
) const
{
    auto a = addrTime.find(addr);
    if (a != addrTime.end())
        return  a->second;
    else
        return TASK_TIME {};
}

/**
 * Clear
 * @param expiration if NULL, clear all
 */
void TimeAddrSet::clear(
    TASK_TIME *expiration
)
{
    if (!expiration) {
        addrTime.clear();
        timeAddr.clear();
    } else {
        for (auto it(timeAddr.begin()); it != timeAddr.end();) {
            if (it->first.time_since_epoch() >= expiration->time_since_epoch()) {
                auto f = addrTime.find(it->second);
                if (f != addrTime.end())
                    addrTime.erase(f);
                it = timeAddr.erase(it);
            } else
                it++;
        }
    }
}

size_t TimeAddrSet::size() const
{
    return addrTime.size();
}