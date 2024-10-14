#ifndef TLNS_STDOUT_BRIDGE_H
#define TLNS_STDOUT_BRIDGE_H

#include "lorawan/bridge/app-bridge.h"

/**
 * App bridge example
 */
class FileJsonBridge : public AppBridge {
protected:
    std::string fileName;
    std::fstream *strm;
public:
    FileJsonBridge() = default;
    virtual ~FileJsonBridge() = default;
    void onPayload(
        const void *dispatcher,
        const MessageQueueItem *messageItem,
        bool decoded,
        bool micMatched
    ) override;
    void init(
        const std::string& option,
        const std::string& option2,
        const void *option3
    ) override;
    void done() override;
};

EXPORT_SHARED_C_FUNC AppBridge* makeBridge2();

#endif //TLNS_STDOUT_BRIDGE_H
