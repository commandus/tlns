#include <iostream>
#include "lorawan/bridge/stdout-bridge.h"
#include "lorawan/lorawan-string.h"

void StdoutBridge::onPayload(
    const void *dispatcher,
    const MessageQueueItem *messageItem,
    bool decoded
)
{
    if (messageItem) {
        if (decoded)
            std::cout << messageItem->toString() << std::endl;
        else
            std::cerr << "Payload size: " << messageItem->radioPacket.payloadSize << " FCnt: "
                      << (int) messageItem->radioPacket.data.uplink.fcnt
                      << " addr: " << DEVADDR2string(messageItem->radioPacket.data.uplink.devaddr)
                      << " key: " << KEY2string(messageItem->task.deviceId.nwkSKey)
                      << " MIC: " << MIC2String(messageItem->radioPacket.mic(messageItem->task.deviceId.nwkSKey))
                      << std::endl;
    }
}

void StdoutBridge::init(
    const std::string& option,
    const std::string& option2,
    const void *option3
)
{

}

void StdoutBridge::done()
{

}

EXPORT_SHARED_C_FUNC AppBridge* makeBridge1()
{
    return new StdoutBridge;
}
