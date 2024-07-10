#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <map>
#include <set>
#include <ostream>

#include "message-queue-item.h"
#include "lorawan/proto/gw/gw.h"
#include "lorawan/task/task-time-addr.h"

class TaskSocket;
class MessageTaskDispatcher;

class MessageQueue {
protected:
    MessageTaskDispatcher *dispatcher;                          ///< parent dispatcher
public:
    std::map <DEVADDR, MessageQueueItem> receivedMessages;      ///< data packets received from devices
    std::map <JOIN_REQUEST_FRAME, MessageQueueItem> joins;      ///< join packets

    TimeAddrSet time2ResponseAddr;

    MessageQueue();
    virtual ~MessageQueue();
    void step();
    /**
     * Set parent dispatcher
     * @param aDispatcher
     */
    void setDispatcher(
        MessageTaskDispatcher *aDispatcher
    );

    MessageQueueItem *get(const DEVADDR &addr);
    MessageQueueItem *get(const JOIN_REQUEST_FRAME &addr);
    void put(
        TASK_TIME time,
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
        TASK_TIME time,
        TaskSocket *taskSocket,
        const struct sockaddr *gwAddr,
        const char *buffer,
        size_t size
    );

    bool put(
        TASK_TIME time,
        GwPushData & pushData
    );
    void rm(
        const DEVADDR &addr
    );
    /**
     * Find item by address
     * @param devAddr devioce address
     * @return null if not found
     */
    MessageQueueItem *findByDevAddr(
        const DEVADDR *devAddr
    );
    MessageQueueItem *findByJoinRequest(
        const JOIN_REQUEST_FRAME *joinRequestFrame
    );
    /**
     * Print out debug information to specified stream
     * @param strm stream to output
     */
    void printStateDebug(
        std::ostream &strm,
        TASK_TIME now
    ) const;

    /**
     * Clear old messages
     * @param since time to delete from
     * @return count of removed items
     */
    size_t clearOldMessages(TASK_TIME since);
};

#endif
