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
            std::cerr << messageItem->toString() << std::endl;
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
