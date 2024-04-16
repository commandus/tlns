#include <algorithm>

#include "message-queue.h"
#include "message-task-dispatcher.h"

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
    auto f = items.find(addr);
    if (f != items.end()) {
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
        auto f = items.find(*addr);
        if (f != items.end()) {
            // update metadata
            f->second.metadata[gwId] = metadata;
        } else {
            MessageQueueItem qi;
            qi.metadata[gwId] = metadata;
            auto i = items.insert(std::pair<DEVADDR, MessageQueueItem>(*addr, qi));
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
    items.insert(std::pair<DEVADDR, MessageQueueItem>(*a, qi));
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
    auto f = items.find(*addr);
    bool isSame = (f != items.end()) && (f->second.radioPacket == pushData.rxData);
    if (isSame) {
        // update metadata
        f->second.metadata[pushData.rxMetadata.gatewayId] = pushData.rxMetadata;
    } else {
        qi.metadata[pushData.rxMetadata.gatewayId] = pushData.rxMetadata;
        qi.radioPacket = pushData.rxData;
        auto i = items.insert(std::pair<DEVADDR, MessageQueueItem>(*addr, qi));
    }
    return !isSame;
}

void MessageQueue::rm(
    const DEVADDR &addr
)
{
    auto f = items.find(addr);
    if (f != items.end())
        items.erase(f);
}

MessageQueueItem *MessageQueue::findByDevAddr(
    const DEVADDR *devAddr
)
{
    auto f = std::find_if(items.begin(), items.end(), [devAddr] (const std::pair<DEVADDR, MessageQueueItem> &v) {
        return v.first == *devAddr;
    } );
    if (f == items.end())
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
