#include <iostream>
#include "lorawan/bridge/file-json-bridge.h"
#include "lorawan/lorawan-string.h"

void FileJsonBridge::onPayload(
    const void* dispatcher,   // MessageTaskDispatcher*
    const MessageQueueItem *messageItem, // network identity, gateway identifier and metadata etc.
    const char *value,
    size_t size
)
{
    if (messageItem)
        std::cout << "Message " << messageItem->toString() << std::endl;
    std::cout << "Payload " << hexString(value, size) << std::endl;
}

void FileJsonBridge::init(
    const std::string& option,
    const void *option2
)
{
    fileName = option;
}

EXPORT_SHARED_C_FUNC AppBridge* makeStdoutBridge()
{
    return new FileJsonBridge;
}