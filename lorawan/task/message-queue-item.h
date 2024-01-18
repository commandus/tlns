#ifndef MESSAGE_QUEUE_ITEM_H_
#define MESSAGE_QUEUE_ITEM_H_

#include <map>

#include "task-platform.h"
#include "task-descriptor.h"
#include "lorawan/lorawan-packet-storage.h"

class MessageQueue;

class MessageQueueItem {
public:
    MessageQueue *queue;                    ///< pointer to collection owns item

    TASK_TIME firstGatewayReceived;         ///< receiving time of the first received packet (no matter which gateway is first)
    LorawanPacketStorage radioPacket;       ///< radio packet
    std::map <uint64_t, SEMTECH_PROTOCOL_METADATA> metadata;   ///< radio metadata sent by each gateway. Metadata describes receiving conditions such as signal power, signal/noise ratio etc.
    
    TaskDescriptor task;                    ///< corresponding task

    MessageQueueItem();
    MessageQueueItem(MessageQueue *owner, const TASK_TIME& time);
    MessageQueueItem(const MessageQueueItem& value);

    void setQueue(MessageQueue *value);

    bool expired(const TASK_TIME &since);

    std::string toString() const;
    /**
     * Return NULL if no address is provided
     * @return
     */
    const DEVADDR *getAddr() const;
};

#endif
