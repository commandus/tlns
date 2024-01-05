#include "task-platform.h"
#include "task-descriptor.h"
#include "lorawan/lorawan-packet-storage.h"

class MessageQueue;

class MessageQueueItem {
public:
    MessageQueue *queue;
    TASK_TIME firstGatewayReceived;         ///< receiving time of the first received packet (no matter which gateway is first)
    LorawanPacketStorage payload;           ///< radio packet
    TaskDescriptor task;                    ///< corresponding task

    bool expired(const TASK_TIME &since);
};
