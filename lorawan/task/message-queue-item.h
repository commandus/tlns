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
    LORAWAN_MESSAGE_STORAGE radioPacket;    ///< radio packet
    std::map <uint64_t, SEMTECH_PROTOCOL_METADATA_RX> metadata;   ///< radio metadata sent by each gateway. Metadata describes receiving conditions such as signal power, signal/noise ratio etc.
    TaskDescriptor task;                    ///< corresponding task

    MessageQueueItem();
    MessageQueueItem(MessageQueue *owner, const TASK_TIME& time);
    MessageQueueItem(const MessageQueueItem& value);

    void setQueue(MessageQueue *value);

    /**
     * Return Check does item expired since time
     * @param since current time
     * @return true if item expired
     */
    bool expired(const TASK_TIME &since);

    /**
     * Serialize item to print out in debug log
     * @return serialize item
     */
    std::string toString() const;
    /**
     * Return network address
     * Return NULL if no address is provided (radio packet is JOIN)
     * @return pointer to device address
     */
    const DEVADDR* getAddr() const;

    /**
     * Return JOIN request frame (used as an identifier)
     * Return NULL radio packet is data packet (not Join request)
     * @return pointer to Join request frame
     */
    const JOIN_REQUEST_FRAME* getJoinRequestFrame() const;
    /**
     * Return gateway MAC address as int with best SNR
     * @param retvalLsnr if provided, return SNR
     * @return 0 if not found
     */
    uint64_t getBestGatewayAddress(
        float *retvalLsnr
    ) const;

};

#endif
