#include <iostream>
#include "lorawan/bridge/stdout-bridge.h"
#include "lorawan/lorawan-string.h"

void StdoutBridge::onPayload(
    const void* dispatcher,   // MessageTaskDispatcher*
    const MessageQueueItem *messageItem, // network identity, gateway identifier and metadata etc.
    const char *value,
    size_t size
)
{
    if (messageItem)
        std::cout << messageItem->toString() << " "
            << hexString(value, size) << std::endl;
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

EXPORT_SHARED_C_FUNC AppBridge* makeBridge()
{
    return new StdoutBridge;
}
