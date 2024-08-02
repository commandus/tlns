#include <iostream>
#include "lorawan/bridge/stdout-bridge.h"
#include "lorawan/lorawan-string.h"

void StdoutBridge::onPayload(
    const void* dispatcher,   // MessageTaskDispatcher*
    const MessageQueueItem *item, // network identity, gateway identifier and metadata etc.
    const char *value,
    size_t size
)
{
    if (item)
        std::cout << "Payload " << item->toString() << std::endl;
    std::cout << "Payload " << hexString(value, size) << std::endl;
}
