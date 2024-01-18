#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <map>

#include "message-queue-item.h"

class MessageTaskDispatcher;

class MessageQueue {
protected:
    MessageTaskDispatcher *dispatcher;
public:
    std::map <DEVADDR, MessageQueueItem> items;
    MessageQueue();
    virtual ~MessageQueue();
    void step();
    void setDispatcher(
        MessageTaskDispatcher *aDispatcher
    );

    MessageQueueItem *get(const DEVADDR &addr);
    void put(
        const LorawanPacketStorage &radioPacket,
        uint64_t gwId,
        const SEMTECH_PROTOCOL_METADATA &metadata
    );
    void rm(const DEVADDR &addr);
    MessageQueueItem *findByDevAddr(const DEVADDR *devAddr);
};

#endif
