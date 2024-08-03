#ifndef TLNS_APP_BRIDGE_H
#define TLNS_APP_BRIDGE_H

#include <cstddef>
#include "lorawan/task/message-queue-item.h"
#include "lorawan/helper/plugin-helper.h"

/**
 * Abstract class connects to
 * @see ServiceClient
 * @see PluginClient
 */
class AppBridge {
public:
    AppBridge() = default;
    virtual ~AppBridge() = default;
    virtual void onPayload(
        const void* dispatcher,   // MessageTaskDispatcher*
        const MessageQueueItem *item, // network identity, gateway identifier and metadata etc.
        const char *value,
        size_t size
    ) = 0;
    virtual void init(
        const std::string& option,
        const void *option2
    ) = 0;
};

#endif
