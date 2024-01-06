#include <chrono>
#include "message-queue-item.h"

#define DEF_MESSAGE_EXPIRATION_SEC  60

MessageQueueItem::MessageQueueItem(
    MessageQueue * ownerQueue,
    const TASK_TIME& time
)
    : queue(ownerQueue), firstGatewayReceived(time)
{

}

MessageQueueItem::MessageQueueItem(
    const MessageQueueItem& value
)
    : queue(value.queue), firstGatewayReceived(value.firstGatewayReceived)
{

}

bool MessageQueueItem::expired(
    const TASK_TIME &since
)
{
    return (std::chrono::duration_cast<std::chrono::seconds>(since - firstGatewayReceived).count() > DEF_MESSAGE_EXPIRATION_SEC);
}
