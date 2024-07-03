#include <algorithm>

#include "message-queue.h"
#include "message-task-dispatcher.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-date.h"

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


MessageQueue::MessageQueue()
{
}

MessageQueue::~MessageQueue() = default;

void MessageQueue::step()
{

}

void MessageQueue::setDispatcher(
    MessageTaskDispatcher *aDispatcher
) {
    dispatcher = aDispatcher;
}

MessageQueueItem *MessageQueue::get (
    const DEVADDR &addr
)
{
    auto f = receivedMessages.find(addr);
    if (f != receivedMessages.end()) {
        return &f->second;
    }
    return nullptr;
}

MessageQueueItem *MessageQueue::get (
    const JOIN_REQUEST_FRAME &join
)
{
    auto f = joins.find(join);
    if (f != joins.end()) {
        return &f->second;
    }
    return nullptr;
}

void MessageQueue::put(
    const LORAWAN_MESSAGE_STORAGE &radioPacket,
    uint64_t gwId,
    const SEMTECH_PROTOCOL_METADATA_RX &metadata
)
{
    auto addr = radioPacket.getAddr();
    if (addr) {
        auto f = receivedMessages.find(*addr);
        if (f != receivedMessages.end()) {
            // update metadata
            f->second.metadata[gwId] = metadata;
        } else {
            MessageQueueItem qi;
            qi.metadata[gwId] = metadata;
            auto i = receivedMessages.insert(std::pair<DEVADDR, MessageQueueItem>(*addr, qi));
        }
    } else {
        // Join request
        const JOIN_REQUEST_FRAME *jr = radioPacket.getJoinRequest();
        if (jr) {

        }
    }
}

bool MessageQueue::put(
    TaskSocket *taskSocket,
    const struct sockaddr *gwAddr,
    const char *buffer,
    size_t size
) {
    MessageQueueItem qi;
    qi.task.stage = TASK_STAGE_GATEWAY_REQUEST;
    const DEVADDR *a = qi.getAddr();
    // TODO parse buffer
    receivedMessages.insert(std::pair<DEVADDR, MessageQueueItem>(*a, qi));
    return true;
}

/**
 * Return true if first packet added, false if it is from another gateway (duplicate)
 * @param pushData
 * @return
 */
bool MessageQueue::put(
    GwPushData & pushData
)
{
    MessageQueueItem qi;
    qi.task.stage = TASK_STAGE_GATEWAY_REQUEST;
    const DEVADDR *addr = pushData.rxData.getAddr();
    auto f = receivedMessages.find(*addr);
    bool isSame = (f != receivedMessages.end()) && (f->second.radioPacket == pushData.rxData);
    if (isSame) {
        // update metadata
        f->second.metadata[pushData.rxMetadata.gatewayId] = pushData.rxMetadata;
    } else {
        qi.metadata[pushData.rxMetadata.gatewayId] = pushData.rxMetadata;
        qi.radioPacket = pushData.rxData;
        auto i = receivedMessages.insert(std::pair<DEVADDR, MessageQueueItem>(*addr, qi));
    }
    return !isSame;
}

void MessageQueue::rm(
    const DEVADDR &addr
)
{
    auto f = receivedMessages.find(addr);
    if (f != receivedMessages.end())
        receivedMessages.erase(f);
}

MessageQueueItem *MessageQueue::findByDevAddr(
    const DEVADDR *devAddr
)
{
    auto f = std::find_if(receivedMessages.begin(), receivedMessages.end(), [devAddr] (const std::pair<DEVADDR, MessageQueueItem> &v) {
        return v.first == *devAddr;
    } );
    if (f == receivedMessages.end())
        return nullptr;
    else
        return &f->second;
}

MessageQueueItem *MessageQueue::findByJoinRequest(
    const JOIN_REQUEST_FRAME *joinRequestFrame
) {
    auto f = std::find_if(joins.begin(), joins.end(), [joinRequestFrame] (const std::pair<JOIN_REQUEST_FRAME, MessageQueueItem> &v) {
        auto a = v.second.getJoinRequestFrame();
        if (!a)
            return false;
        return *a == *joinRequestFrame;
    } );
    if (f == joins.end())
        return nullptr;
    else
        return &f->second;
}

void MessageQueue::printStateDebug(
    std::ostream &strm
) const
{
    TASK_TIME now = std::chrono::system_clock::now();

    strm << "Time " << taskTime2string(now) << "\n";
    // data packets received from devices
    strm << receivedMessages.size() << " received messages\n";
    for (auto m : receivedMessages) {
        strm << DEVADDR2string(m.first) << "\t" << taskTime2string(m.second.firstGatewayReceived) << "\n";
    }

    // Device addresses wait for ACK or response sorted by time
    strm << "Server waiting for " << time2ResponseAddr.size() << " messages from the gateways before sending response:\n";
    for (auto t : time2ResponseAddr) {
        strm << DEVADDR2string(t.addr) << "\t"
            << taskTime2string(t.startTime) << "\n";
    }
    strm << "Waiting time " << waitTimeMicroseconds(now) << " us\n";
}

long MessageQueue::waitTimeMicroseconds(
    TASK_TIME since
) const
{
    auto ta = time2ResponseAddr.begin();
    if (ta != time2ResponseAddr.end())
        return -1;
    auto delta = std::chrono::duration_cast<std::chrono::microseconds>(since - ta->startTime);
    return delta.count();
}
