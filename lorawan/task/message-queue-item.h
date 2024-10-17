#ifndef MESSAGE_QUEUE_ITEM_H_
#define MESSAGE_QUEUE_ITEM_H_

#include <map>

#include "task-platform.h"
#include "task-descriptor.h"
#include "lorawan/lorawan-packet-storage.h"
#include "task-socket.h"

class MessageQueue;
class ProtoGwParser;

class GatewayMetadata {
public:
    SEMTECH_PROTOCOL_METADATA_RX rx;
    const TaskSocket *taskSocket;
    struct sockaddr addr;                         ///< uplink: gateway network address (if any) where packet was sent from
    ProtoGwParser *parser;                        ///< uplink: pointer to parser has been used (can be NULL)
    std::string toJsonString() const;
};

class MessageQueueItem {
public:
    MessageQueue *queue;                            ///< pointer to collection owns item
    TASK_TIME tim;                                  ///< uplink: receiving time of the first received packet (no matter which gateway is first). Downlink- time to send
    LORAWAN_MESSAGE_STORAGE radioPacket;            ///< radio packet
    std::map <uint64_t, GatewayMetadata> metadata;  ///< radio metadata sent by each gateway. Metadata describes receiving conditions such as signal power, signal/noise ratio etc.
    TaskDescriptor task;                            ///< corresponding task

    MessageQueueItem();
    MessageQueueItem(MessageQueue *owner, const TASK_TIME& time);
    MessageQueueItem(MessageQueue *owner, const TASK_TIME& time, ProtoGwParser *parser);
    MessageQueueItem(const MessageQueueItem& value);

    void setQueue(MessageQueue *value);

    /**
     * Return Check does item expired since time
     * @param since current time
     * @return true if item expired
     */
    bool expired(const TASK_TIME &since) const;

    /**
     * Serialize item to print out in debug log
     * @return serialize item
     */
    std::string toString() const;
    /**
     * Serialize item to JSON
     * @return serialize item
     */
    std::string toJsonString() const;
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
     * @param retValMetadata gateway metadata
     * @return 0 if not found
     */
    uint64_t getBestGatewayAddress(
        GatewayMetadata &retValMetadata
    ) const;

    bool needConfirmation() const;
};

#endif
