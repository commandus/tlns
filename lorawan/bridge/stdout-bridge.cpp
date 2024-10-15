#include <iostream>
#include "lorawan/bridge/stdout-bridge.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-error.h"

StdoutBridge::StdoutBridge() = default;

void StdoutBridge::onPayload(
    const void *dispatcher,
    const MessageQueueItem *messageItem,
    bool decoded,
    bool micMatched
)
{
    if (messageItem) {
        if (decoded)
            std::cout << messageItem->toString() << std::endl;
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

void StdoutBridge::onSend(
    const void *dispatcher,
    const MessageQueueItem *item,
    int code
)
{
    if (item)
        std::cerr << "Sent " << item->toString() << " with code " << code << std::endl;
}

EXPORT_SHARED_C_FUNC AppBridge* makeBridge1()
{
    return new StdoutBridge;
}
