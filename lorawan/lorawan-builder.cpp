#include "lorawan/lorawan-builder.h"
#include "base64/base64.h"

MessageBuilder::MessageBuilder(
    const TaskDescriptor &aTaskDescriptor
)
    : msg {}, taskDescriptor(aTaskDescriptor)
{

}

size_t MessageBuilder::get(
    void *buffer,
    size_t size
) const
{
    return msg.toArray(buffer, size, &taskDescriptor.deviceId);
}

size_t MessageBuilder::size() const
{
    return msg.toArray(nullptr, 0, &taskDescriptor.deviceId);
}

std::string MessageBuilder::base64() const
{
    char buffer[300];
    auto sz = msg.toArray(buffer, sizeof(buffer), &taskDescriptor.deviceId);
    return base64_encode(std::string(buffer, sz));
}

ConfirmationMessage::ConfirmationMessage(
    const LORAWAN_MESSAGE_STORAGE &message2confirm,
    const TaskDescriptor &taskDescriptor
)
    : MessageBuilder(taskDescriptor)
{
    msg = message2confirm;
    // set direction
    msg.mhdr.f.mtype = MTYPE_CONFIRMED_DATA_DOWN;
    // remove options by settings size to 0
    msg.data.downlink.f.foptslen = 0;
    // remove payload by settings size to 0
    msg.payloadSize = 0;
    // set ACK bit according to LoRaWAN 1.1 specification 4.3.1.2 Message acknowledge bit and acknowledgement procedure (ACK in FCtrl)
    msg.data.downlink.f.ack = 1;
}

// DownlinkMessage (from the server to the end-device)

DownlinkMessage::DownlinkMessage(
    const TaskDescriptor &taskDescriptor,    // contain NetworkIdentity and best gateway address
    uint8_t fport,
    const void *payload, // up to 255 bytes, can be NULL
    uint8_t payloadSize,
    const void *fopts, // up to 15 bytes, can be NULL
    uint8_t foptsSize
)
    : MessageBuilder(taskDescriptor)
{
    // set direction
    msg.mhdr.f.mtype = MTYPE_CONFIRMED_DATA_DOWN;
    // MAC options
    msg.data.downlink.f.foptslen = foptsSize;
    // payload
    msg.payloadSize = payloadSize;
    msg.data.downlink.devaddr = taskDescriptor.deviceId.devaddr;
    msg.data.downlink.setFport(fport);
    if (fopts && foptsSize > 0) {
        msg.data.downlink.setFOpts(fopts, foptsSize);
    }
    if (payload && payloadSize > 0) {
        msg.data.downlink.setPayload(payload, payloadSize);
    }
}
