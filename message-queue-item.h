#include <string>

#include "task-platform.h"
#include "task-descriptor.h"

class MessageQueue;

class MessageQueueItem {
public:
    MessageQueue *queue;
    TASK_TIME firstGatewayReceived;         ///< receiving time of the first received packet (no matter which gateway is first)
    std::string payload;                    ///< radio packet
    TaskDescriptor task;                    ///< corresponding task

    bool expired(const TASK_TIME &since);
};
