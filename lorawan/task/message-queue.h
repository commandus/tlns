#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <map>
#include <set>
#include <ostream>

#include "lorawan/proto/gw/gw.h"
#include "lorawan/task/task-time-addr.h"

class TaskSocket;
class MessageTaskDispatcher;
class MessageQueueItem;
class ProtoGwParser;

class MessageQueue {
protected:
    MessageTaskDispatcher *dispatcher;                          ///< parent dispatcher
public:
    std::map <DEVADDR, MessageQueueItem> uplinkMessages;        ///< data packets received from devices
    std::map <JOIN_REQUEST_FRAME, MessageQueueItem> joins;      ///< join packets
    std::map <DEVADDR, MessageQueueItem> downlinkMessages;      ///< data packets ready to send to devices, one message per device

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

    MessageQueueItem *getUplink(const DEVADDR &addr);
    MessageQueueItem *getJoinRequest(const JOIN_REQUEST_FRAME &addr);
    MessageQueueItem *getDownlink(const DEVADDR &addr);
    void putUplink(
        const TASK_TIME& time,
        const TaskSocket *taskSocket,
        const LORAWAN_MESSAGE_STORAGE &radioPacket,
        const struct sockaddr &addr,
        uint64_t gwId,
        const SEMTECH_PROTOCOL_METADATA_RX &metadata,
        ProtoGwParser *parser
    );
    bool putUplink(
        const TASK_TIME& time,
        const TaskSocket *taskSocket,
        const struct sockaddr &srcAddr,
        GwPushData &pushData,
        ProtoGwParser *parser
    );
    void putDownlink(
        const TASK_TIME& time,
        const DEVADDR &devAddr,
        const TaskSocket *taskSocket,
        const LORAWAN_MESSAGE_STORAGE &radioPacket,
        const struct sockaddr &addr,
        uint64_t gwId,
        const SEMTECH_PROTOCOL_METADATA_TX &metadata,
        ProtoGwParser *parser
    );
    void rmUplink(
        const DEVADDR &addr
    );
    void rmDownlink(
        const DEVADDR &addr
    );
    /**
     * Find item by address
     * @param devAddr devioce address
     * @return null if not found
     */
    MessageQueueItem *findUplink(
        const DEVADDR *devAddr
    );
    MessageQueueItem *findJoinRequest(
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
    size_t clearOldUplinkMessages(TASK_TIME since);
    size_t clearOldDownlinkMessages(TASK_TIME since);
};

#endif
