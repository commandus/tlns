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
    std::map <uint64_t, SEMTECH_PROTOCOL_METADATA> metadatas;   ///< radio metadatas sent by each gateway. Metadata describes receiving conditions such as signal power, signal/noise ratio etc.
    
    TaskDescriptor task;                    ///< corresponding task

    MessageQueueItem(MessageQueue *owner, const TASK_TIME& time);

    bool expired(const TASK_TIME &since);
};
