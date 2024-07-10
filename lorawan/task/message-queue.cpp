#include <algorithm>
#include <iomanip>

#include "message-queue.h"
#include "message-task-dispatcher.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-date.h"

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
    TASK_TIME time,
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
            MessageQueueItem qi(this, time);
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
    TASK_TIME time,
    TaskSocket *taskSocket,
    const struct sockaddr *gwAddr,
    const char *buffer,
    size_t size
) {
    MessageQueueItem qi(this, time);
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
    TASK_TIME time,
    GwPushData &pushData
)
{
    MessageQueueItem qi(this, time);
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
    std::ostream &strm,
    TASK_TIME now
) const
{
    strm << "Time " << taskTime2string(now) << "\n";
    // data packets received from devices
    strm << receivedMessages.size() << " received messages\n";
    for (auto m : receivedMessages) {
        strm << DEVADDR2string(m.first) << "\t" << taskTime2string(m.second.firstGatewayReceived) << "\n";
    }

    // Device addresses wait for ACK or response sorted by time
    strm << "Server waiting for " << time2ResponseAddr.size() << " messages from the gateways before sending response:\n";
    for (auto t : time2ResponseAddr.timeAddr) {
        strm << DEVADDR2string(t.second) << "\t"
            << taskTime2string(t.first) << "\n";
    }
    strm << "Waiting time " << std::fixed << std::setprecision(6) <<
        time2ResponseAddr.waitTimeForAllGatewaysInMicroseconds(now) / 1000000. << " s\n";
}

/**
 * Clear old messages
 * @param since time to delete from
 * @return count of removed items
 */
size_t MessageQueue::clearOldMessages(
    TASK_TIME since
)
{
    for (auto m(receivedMessages.begin()); m != receivedMessages.end();) {
        if (m->second.firstGatewayReceived > since) {
            m = receivedMessages.erase(m);
        } else
            m++;
    }
}
