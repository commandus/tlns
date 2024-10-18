#include <algorithm>
#include <iomanip>

#include "message-queue.h"
#include "message-task-dispatcher.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-date.h"

MessageQueue::MessageQueue()
    : dispatcher(nullptr)
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

MessageQueueItem *MessageQueue::getUplink (
    const DEVADDR &addr
)
{
    auto f = uplinkMessages.find(addr);
    if (f != uplinkMessages.end()) {
        return &f->second;
    }
    return nullptr;
}

MessageQueueItem *MessageQueue::getJoinRequest (
    const JOIN_REQUEST_FRAME &addr
)
{
    auto f = joins.find(addr);
    if (f != joins.end()) {
        return &f->second;
    }
    return nullptr;
}

MessageQueueItem *MessageQueue::getDownlink(
    const DEVADDR &addr
)
{
    auto f = downlinkMessages.find(addr);
    if (f != downlinkMessages.end()) {
        return &f->second;
    }
    return nullptr;
}

void MessageQueue::putUplink(
    const TASK_TIME& time,
    const TaskSocket *taskSocket,
    const LORAWAN_MESSAGE_STORAGE &radioPacket,
    const struct sockaddr &addr,
    uint64_t gwId,
    const SEMTECH_PROTOCOL_METADATA_RX &metadata,
    ProtoGwParser *parser
)
{
    auto loraAddr = radioPacket.getAddr();
    if (loraAddr) {
        auto f = uplinkMessages.find(*loraAddr);
        if (f != uplinkMessages.end()) {
            // update metadata
            f->second.metadata[gwId] = { metadata, taskSocket, addr };
        } else {
            MessageQueueItem qi(this, time, parser);
            qi.metadata[gwId] = { metadata, taskSocket, addr };
            auto i = uplinkMessages.insert(std::pair<DEVADDR, MessageQueueItem>(*loraAddr, qi));
        }
    } else {
        // Join request
        const JOIN_REQUEST_FRAME *jr = radioPacket.getJoinRequest();
        if (jr) {

        }
    }
}

/**
 * Return true if first packet added, false if it is from another gateway (duplicate)
 * @param pushData
 * @return
 */
bool MessageQueue::putUplink(
    const TASK_TIME &time,
    const TaskSocket *taskSocket,
    const struct sockaddr &srcAddr,
    GwPushData &pushData,
    ProtoGwParser *parser
)
{
    MessageQueueItem qi(this, time, parser);
    qi.task.stage = TASK_STAGE_GATEWAY_REQUEST;
    const DEVADDR *addr = pushData.rxData.getAddr();
    if (!addr)
        return false;
    auto f = uplinkMessages.find(*addr);
    bool isSame = (f != uplinkMessages.end()) && (f->second.radioPacket == pushData.rxData);
    if (isSame) {
        // update metadata
        f->second.metadata[pushData.rxMetadata.gatewayId] = { pushData.rxMetadata, taskSocket, srcAddr, parser };
    } else {
        if (dispatcher && dispatcher->identityClient) {
            // getUplink device identity
            DEVICEID did;
            dispatcher->identityClient->svcIdentity->get(did, *addr);
            qi.task.deviceId.set(*addr, did);
        }

        qi.metadata[pushData.rxMetadata.gatewayId] = { pushData.rxMetadata, taskSocket, srcAddr, parser };
        qi.task.gatewayId = pushData.rxMetadata.gatewayId;
        qi.task.deviceId.devaddr = *addr;
        qi.task.repeats = 0;
        qi.task.errorCode = 0;
        qi.radioPacket = pushData.rxData;
        auto i = uplinkMessages.insert(std::pair<DEVADDR, MessageQueueItem>(*addr, qi));
    }
    return !isSame;
}

void MessageQueue::putDownlink(
    const TASK_TIME& time,
    const DEVADDR &devAddr,
    const TaskSocket *taskSocket,
    const LORAWAN_MESSAGE_STORAGE &radioPacket,
    const struct sockaddr &addr,
    uint64_t gwId,
    const SEMTECH_PROTOCOL_METADATA_TX &metadata,
    ProtoGwParser *parser
)
{
    auto f = downlinkMessages.find(devAddr);
    if (f != downlinkMessages.end()) {
        // update metadata
        // f->second.metadata[gwId] = { metadata, taskSocket, addr };
    } else {
        MessageQueueItem qi(this, time, parser);
        // qi.metadata[gwId] = { metadata, taskSocket, addr };
        auto i = uplinkMessages.insert(std::pair<DEVADDR, MessageQueueItem>(devAddr, qi));
    }
}

void MessageQueue::rmUplink(
    const DEVADDR &addr
)
{
    auto f = uplinkMessages.find(addr);
    if (f != uplinkMessages.end())
        uplinkMessages.erase(f);
}

void MessageQueue::rmDownlink(
    const DEVADDR &addr
)
{
    auto f = downlinkMessages.find(addr);
    if (f != downlinkMessages.end())
        downlinkMessages.erase(f);
}

MessageQueueItem *MessageQueue::findUplink(
    const DEVADDR *devAddr
)
{
    auto f = std::find_if(uplinkMessages.begin(), uplinkMessages.end(), [devAddr] (const std::pair<DEVADDR, MessageQueueItem> &v) {
        return v.first == *devAddr;
    } );
    if (f == uplinkMessages.end())
        return nullptr;
    else
        return &f->second;
}

MessageQueueItem *MessageQueue::findJoinRequest(
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
    strm << uplinkMessages.size() << " received uplink messages\n";
    for (const auto& m : uplinkMessages) {
        strm << DEVADDR2string(m.first) << "\t" << taskTime2string(m.second.tim) << "\n";
    }
    // data packets ready to send to devices
    strm << downlinkMessages.size() << " downlink messages\n";
    for (const auto& m : downlinkMessages) {
        strm << DEVADDR2string(m.first) << "\t" << taskTime2string(m.second.tim) << "\n";
    }

    // Device addresses wait for ACK or response sorted by time
    strm << "Server waiting for " << time2ResponseAddr.size() << " messages from the gateways before sending response:\n";
    for (const auto& t : time2ResponseAddr.timeAddr) {
        strm << DEVADDR2string(t.second) << "\t"
            << taskTime2string(t.first) << "\n";
    }
    strm << "Waiting time " << std::fixed << std::setprecision(6) <<
        time2ResponseAddr.waitTimeForAllGatewaysInMicroseconds(now) / 1000000. << " s" << std::endl;
}

/**
 * Clear old messages
 * @param since time to delete from
 * @return count of removed items
 */
size_t MessageQueue::clearOldUplinkMessages(
    TASK_TIME since
)
{
    size_t r = 0;
    for (auto m(uplinkMessages.begin()); m != uplinkMessages.end();) {
        if (m->second.tim < since) {
            m = uplinkMessages.erase(m);
            r++;
        } else
            m++;
    }
    return r;
}

/**
 * Clear old messages
 * @param since time to delete from
 * @return count of removed items
 */
size_t MessageQueue::clearOldDownlinkMessages(
    TASK_TIME since
)
{
    size_t r = 0;
    for (auto m(downlinkMessages.begin()); m != downlinkMessages.end();) {
        if (m->second.tim < since) {
            m = downlinkMessages.erase(m);
            r++;
        } else
            m++;
    }
    return r;
}
