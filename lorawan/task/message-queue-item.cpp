#include <chrono>
#include <sstream>
#include "message-queue-item.h"
#include "lorawan/lorawan-date.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/ip-address.h"
#include "lorawan/proto/gw/proto-gw-parser.h"

#define DEF_MESSAGE_EXPIRATION_SEC  60

std::string GatewayMetadata::toJsonString() const
{
    std::stringstream ss;
    ss << "{\"taskSocket\": " << taskSocket->toJsonString()
        << R"(, "sockAddr": ")" << sockaddr2string(&addr) << "\""
        << ", \"rx\": " << SEMTECH_PROTOCOL_METADATA_RX2string(rx);
    if (parser)
        ss << ", \"protocol\": " << parser->toJsonString();
    ss << "}";
    return ss.str();
}

MessageQueueItem::MessageQueueItem()
    : queue(nullptr), tim(std::chrono::system_clock::now())
{

}

MessageQueueItem::MessageQueueItem(
    MessageQueue * ownerQueue,
    const TASK_TIME& time
)
    : queue(ownerQueue), tim(time)
{

}

MessageQueueItem::MessageQueueItem(
    MessageQueue *ownerQueue,
    const TASK_TIME& time,
    ProtoGwParser *aParser
)
    : queue(ownerQueue), tim(time)
{

}

MessageQueueItem::MessageQueueItem(
    const MessageQueueItem& value
)
    : queue(value.queue), tim(value.tim),
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
) const
{
    return (std::chrono::duration_cast<std::chrono::seconds>(since - tim).count() > DEF_MESSAGE_EXPIRATION_SEC);
}

std::string MessageQueueItem::toString() const
{
    std::stringstream ss;
    std::time_t t = std::chrono::system_clock::to_time_t(tim);
    ss << R"({"received": ")" << time2string(t) << R"(", "radio": )" << radioPacket.toString();
    ss << ", \"gateways\": [";
    for (auto it : metadata) {
        ss << R"({"id": ")" <<  gatewayId2str(it.first)
            << R"(", "lsnr": )" << it.second.rx.lsnr
            << ", \"frequency\": " << freq2string(it.second.rx.freq)
            << ", \"chan\": " << (int) it.second.rx.chan;
        if (it.second.parser)
            ss << ", \"protocol\": " << it.second.parser->toJsonString();
        ss << "}";
    }
    ss << "]}";
    return ss.str();
}

std::string MessageQueueItem::toJsonString() const
{
    std::time_t t = std::chrono::system_clock::to_time_t(tim);
    std::stringstream ss;
    ss << R"({"received": ")" << time2string(t) << "\", \"radio\": " << radioPacket.toString();
    if (radioPacket.payloadSize)
        ss << R"(, "payload": ")" << hexString((const char *) radioPacket.data.downlink.payload(), radioPacket.payloadSize) << "\"";
    if (!metadata.empty()) {
        ss << ", \"gateways\": [";
        bool isFirst = true;
        for (auto it: metadata) {
            if (isFirst)
                isFirst = false;
            else
                ss << ", ";
            ss << R"({"gatewayId": ")" << gatewayId2str(it.first)
                << R"(", "metadata": )" << it.second.toJsonString()
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
    for (const auto & it : metadata) {
        if (it.second.rx.lsnr > f) {
            r = it.first;
            retValMetadata = it.second;
            f = it.second.rx.lsnr;
        }
    }
    return r;
}

bool MessageQueueItem::needConfirmation() const {
    return (this->radioPacket.mhdr.f.mtype == MTYPE_CONFIRMED_DATA_UP)
        || (this->radioPacket.mhdr.f.mtype == MTYPE_CONFIRMED_DATA_DOWN);
}
