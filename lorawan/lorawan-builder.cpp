#include "lorawan/lorawan-builder.h"

MessageBuilder::MessageBuilder(
    const TaskDescriptor &aTaskDescriptor
)
    : msg {}, taskDescriptor(aTaskDescriptor)
{

}

size_t MessageBuilder::get(
    void *buffer,
    size_t size
)
{
    return msg.toArray(buffer, size, &taskDescriptor.deviceId);
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
    msg.packetSize = 0;
    // set ACK bit according to LoRaWAN 1.1 specification 4.3.1.2 Message acknowledge bit and acknowledgement procedure (ACK in FCtrl)
    msg.data.downlink.f.ack = 1;
}
