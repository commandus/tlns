#ifndef TLNS_STDOUT_BRIDGE_H
#define TLNS_STDOUT_BRIDGE_H

#include "lorawan/bridge/app-bridge.h"

/**
 * App bridge example. Write payload to "payload.json" file
 * (if first parameter of init() is empty).
 * This bridge cannot send data to another client.
 */
class FileJsonBridge : public AppBridge {
protected:
    std::string fileName;
    std::fstream *strm;
public:
    FileJsonBridge();
    virtual ~FileJsonBridge() = default;
    void onPayload(
        const void *dispatcher,
        const MessageQueueItem *messageItem,
        bool decoded,
        bool micMatched
    ) override;
    void onSend(
        const void *dispatcher,
        const MessageQueueItem *item,
        int code
    ) override;
    int init(
        const std::string& option,
        const std::string& option2,
        const void *option3
    ) override;
    void done() override;
    const char *name() override;
};

EXPORT_SHARED_C_FUNC AppBridge* makeBridge2();

#endif //TLNS_STDOUT_BRIDGE_H
