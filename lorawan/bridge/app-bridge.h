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
private:
    void *dispatcher;
public:
    AppBridge();
    virtual ~AppBridge() = default;

    void setDispatcher(
        void *aDispatcher
    );

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
     * Common name of the bridge
     * @return name
     */
    virtual const char *name() = 0;

    /**
     * Finalize bridge
     */
    virtual void done() = 0;

    /**
     * Prepare to send FOpts and/or payload to he end-device
     */
    int send(
        const TASK_TIME &tim,
        const DEVADDR &addr,
        void *buffer,
        void *fopts,
        uint8_t fPort,
        uint8_t bufferSize = 0,
        uint8_t foptsSize = 0
    );

    // helpful wrappers

    int sendFOpts(
        const TASK_TIME &tim,
        const DEVADDR &addr,
        void *fopts,
        uint8_t size
    );

    int sendPayload(
        const TASK_TIME &tim,
        const DEVADDR &addr,
        uint8_t fPort,
        void *payload,
        uint8_t size
    );

    int send(
        const TASK_TIME &tim,
        const DEVADDR &addr,
        uint8_t fPort,
        const std::string &payload,
        const std::string &fOpts
    );

    int sendFOpts(
        const TASK_TIME &tim,
        const DEVADDR &addr,
        const std::string &fopts
    );

    int sendPayload(
        const TASK_TIME &tim,
        const DEVADDR &addr,
        uint8_t fPort,
        const std::string &payload
    );

    /**
     * return send status on @param code
     * @param dispatcher duspatcher
     * @param item message in a queue (cen be NULL)
     * @param code send status code. 0- success
     */
    virtual void onSend(
        const void *dispatcher,
        const MessageQueueItem *item,
        int code
    ) = 0;
};

#endif
