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

void MessageQueue::put(
    const LorawanPacketStorage &radioPacket,
    uint64_t gwId,
    const SEMTECH_PROTOCOL_METADATA &metadata
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
    }
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
        auto a = v.second.getAddr();
        if (!a)
            return false;
        return *a == *devAddr;
    } );
    if (f == items.end())
        return nullptr;
    else
        return &f->second;
}
