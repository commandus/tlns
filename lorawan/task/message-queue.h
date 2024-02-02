#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <map>

#include "message-queue-item.h"
#include "lorawan/proto/gw/gw.h"

class TaskSocket;
class MessageTaskDispatcher;

class MessageQueue {
protected:
    MessageTaskDispatcher *dispatcher;
public:
    std::map <DEVADDR, MessageQueueItem> items;
    std::map <JOIN_REQUEST_FRAME, MessageQueueItem> joins;
    MessageQueue();
    virtual ~MessageQueue();
    void step();
    void setDispatcher(
        MessageTaskDispatcher *aDispatcher
    );

    MessageQueueItem *get(const DEVADDR &addr);
    MessageQueueItem *get(const JOIN_REQUEST_FRAME &addr);
    void put(
        const LORAWAN_MESSAGE_STORAGE &radioPacket,
        uint64_t gwId,
        const SEMTECH_PROTOCOL_METADATA_RX &metadata
    );
    /**
     * Packet received from gateway
     * @param taskSocket addr gateway address port gateway port
     * @param gwAddr gateway socket address
     * @param buffer radio packet buffer and JSON metadata
     * @param size buffer size
     * @return false if packet is invalid
     */
    bool put(
        TaskSocket *taskSocket,
        const struct sockaddr *gwAddr,
        const char *buffer,
        size_t size
    );

    bool put(
        GwPushData & pushData
    );
    void rm(const DEVADDR &addr);
    MessageQueueItem *findByDevAddr(const DEVADDR *devAddr);
    MessageQueueItem *findByJoinRequest(const JOIN_REQUEST_FRAME *joinRequestFrame);
};

#endif
