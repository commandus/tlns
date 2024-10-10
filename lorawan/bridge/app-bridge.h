#ifndef TLNS_APP_BRIDGE_H
#define TLNS_APP_BRIDGE_H

#include <cstddef>
#include "lorawan/task/message-queue-item.h"
#include "lorawan/helper/plugin-helper.h"

/**
 * Abstract class connects to application service
 * @see ServiceClient
 * @see PluginClient
 */
class AppBridge {
public:
    AppBridge() = default;
    virtual ~AppBridge() = default;
    virtual void onPayload(
        const void *dispatcher,
        const MessageQueueItem *item,
        bool decoded,
        bool micMatched
    ) = 0;
    virtual void init(
        const std::string& option,
        const std::string& option2,
        const void *option3
    ) = 0;
    virtual void done() = 0;
};

#endif
