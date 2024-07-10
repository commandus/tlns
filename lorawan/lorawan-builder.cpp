#include "lorawan/lorawan-builder.h"

MessageBuilder::MessageBuilder()
    : msg {}
{

}

ConfirmationMessage::ConfirmationMessage(
    const LORAWAN_MESSAGE_STORAGE &message2confirm
)
{
    msg = message2confirm;
    // remove options by settings size to 0
    msg.data.downlink.f.foptslen = 0;
    // remove payload by settings size to 0
    msg.packetSize = 0;
    // set ACK bit according to LoRaWAN 1.1 specification 4.3.1.2 Message acknowledge bit and acknowledgement procedure (ACK in FCtrl)
    msg.data.downlink.f.ack = 1;
}
