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
    /**
     *
     * @param dispatcher Dispatcher. Can be NULL
     * @param item Message in a queue
     * @param decoded true- message decoded, false- no
     * @param micMatched  indicate does MIC matched
     */
    virtual void onPayload(
        const void *dispatcher,
        const MessageQueueItem *item,
        bool decoded,
        bool micMatched
    ) = 0;
    /**
     * Initialize bridge. All options are optional.
     * @param option
     * @param option2
     * @param option3
     */
    virtual void init(
        const std::string& option,
        const std::string& option2,
        const void *option3
    ) = 0;
    /**
     * Finalize bridge
     */
    virtual void done() = 0;
};

#endif
