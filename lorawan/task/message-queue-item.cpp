#include <chrono>
#include <sstream>
#include "message-queue-item.h"
#include "lorawan/lorawan-date.h"
#include "lorawan/lorawan-string.h"

#define DEF_MESSAGE_EXPIRATION_SEC  60

MessageQueueItem::MessageQueueItem()
    : queue(nullptr), firstGatewayReceived(std::chrono::system_clock::now())
{

}

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
    : queue(value.queue), firstGatewayReceived(value.firstGatewayReceived),
        radioPacket(value.radioPacket), metadata(value.metadata), task(value.task)
{
}

void MessageQueueItem::setQueue(
    MessageQueue *value
)
{
    queue = value;
}

bool MessageQueueItem::expired(
    const TASK_TIME &since
)
{
    return (std::chrono::duration_cast<std::chrono::seconds>(since - firstGatewayReceived).count() > DEF_MESSAGE_EXPIRATION_SEC);
}

std::string MessageQueueItem::toString() const
{
    std::stringstream ss;
    std::time_t t = std::chrono::system_clock::to_time_t(firstGatewayReceived);
    ss << time2string(t) << " " << radioPacket.toString();
    for (auto it : metadata) {
        ss << " " <<  gatewayId2str(it.first)
            << ": " << BANDWIDTH2String(it.second.bandwidth);
    }
    return ss.str();
}

const DEVADDR * MessageQueueItem::getAddr() const
{
    if (radioPacket.mhdr.f.mtype >= MTYPE_UNCONFIRMED_DATA_UP
        && radioPacket.mhdr.f.mtype <= MTYPE_CONFIRMED_DATA_DOWN) {
        return &radioPacket.data.downlink.devaddr;
    }
    return nullptr;
}

const JOIN_REQUEST_FRAME * MessageQueueItem::getJoinRequestFrame() const {
    if (radioPacket.mhdr.f.mtype >= MTYPE_JOIN_REQUEST) {
        return &radioPacket.data.joinRequest;
    }
    return nullptr;
}

/**
 * Return gateway MAC address as int with best SNR
 * @param retvalLsnr if provided, return SNR
 * @return 0 if not found
 */
uint64_t MessageQueueItem::getBestGatewayAddress(
    float *retvalLsnr
) const
{
    float f = -3.402823466E+38f;
    uint64_t r = 0;
    for (std::map <uint64_t, SEMTECH_PROTOCOL_METADATA_RX>::const_iterator it(metadata.begin()); it != metadata.end(); it++)
    {
        if (it->second.lsnr > f)
        {
            f = it->second.lsnr;
            r = it->first;
            if (retvalLsnr) {
                *retvalLsnr = f;
            }
        }
    }
    return r;
}
