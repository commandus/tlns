#include "lorawan/task/task-time-addr.h"

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
    auto ta = addrTime.begin();
    bool b = ta != addrTime.end();
    if (b) {
        if (retVal) {
            retVal->addr = ta->first;
            retVal->startTime = ta->second;
        }
        addrTime.erase(ta);
        for (auto t = timeAddr.begin(); t != timeAddr.end(); t++) {
            if (t->second == ta->first) {
                timeAddr.erase(t);
                break;
            }
        }

    }
    return b;
}

long TimeAddrSet::waitTimeMicroseconds(
    TASK_TIME since
) const
{
    auto ta = timeAddr.begin();
    if (ta != timeAddr.end())
        return -1;
    auto delta = std::chrono::duration_cast<std::chrono::microseconds>(since - ta->first);
    return delta.count();
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

    }
}

size_t TimeAddrSet::size() const
{
    return addrTime.size();
}