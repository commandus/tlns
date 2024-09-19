#include <chrono>
#include <sstream>
#include "message-queue-item.h"
#include "lorawan/lorawan-date.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/ip-address.h"

#define DEF_MESSAGE_EXPIRATION_SEC  60

std::string GatewayMetadata::toJsonString() const
{
    std::stringstream ss;
    ss << "{\"taskSocket\": " << taskSocket->toJsonString()
        << ", \"sockAddr\": \"" << sockaddr2string(&addr) << "\""
        << ", \"rx\": " << SEMTECH_PROTOCOL_METADATA_RX2string(rx)
        << "}";
    return ss.str();
}

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
    ss << "{\"time\": " << time2string(t) << "\", \"radio\": " << radioPacket.toString()
        << ", \"metadata\": [";
    for (auto it : metadata) {
        ss << " " <<  gatewayId2str(it.first)
            << ": " << BANDWIDTH2String(it.second.rx.bandwidth);
    }
    ss << "]";
    return ss.str();
}

std::string MessageQueueItem::toJsonString() const
{
    std::time_t t = std::chrono::system_clock::to_time_t(firstGatewayReceived);
    std::stringstream ss;
    ss << "{\"received\": \"" << time2string(t) << "\", \"radio\": " << radioPacket.toString();
    if (radioPacket.payloadSize)
        ss << ", \"payload\": \"" << hexString((const char *) radioPacket.data.downlink.payload(), radioPacket.payloadSize) << "\"";
    if (!metadata.empty()) {
        ss << ", \"gateways\": [";
        bool isFirst = true;
        for (auto it: metadata) {
            if (isFirst)
                isFirst = false;
            else
                ss << ", ";
            ss << "{\"gatewayId\": \"" << gatewayId2str(it.first)
                << "\", \"metadata\": " << it.second.toJsonString()
                << "}";
        }
        ss << "]";
    }
    ss << "}";
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
 * @param retValMetadata RX metadata
 * @return 0 if not found
 */
uint64_t MessageQueueItem::getBestGatewayAddress(
    GatewayMetadata &retValMetadata
) const
{
    float f = -3.402823466E+38f;
    uint64_t r = 0;
    for (std::map <uint64_t, GatewayMetadata>::const_iterator it(metadata.begin()); it != metadata.end(); it++) {
        if (it->second.rx.lsnr > f) {
            r = it->first;
            retValMetadata = it->second;
            f = it->second.rx.lsnr;
        }
    }
    return r;
}

bool MessageQueueItem::needConfirmation() {
    return (this->radioPacket.mhdr.f.mtype == MTYPE_CONFIRMED_DATA_UP)
        || (this->radioPacket.mhdr.f.mtype == MTYPE_CONFIRMED_DATA_DOWN);
}
