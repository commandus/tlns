#ifndef TLNS_STDOUT_BRIDGE_H
#define TLNS_STDOUT_BRIDGE_H

#include "lorawan/bridge/app-bridge.h"

class FileJsonBridge : public AppBridge {
protected:
    std::string fileName;
public:
    FileJsonBridge() = default;
    virtual ~FileJsonBridge() = default;
    void onPayload(
        const void* dispatcher,   // MessageTaskDispatcher*
        const MessageQueueItem *messageItem, // network identity, gateway identifier and metadata etc.
        const char *value,
        size_t size
    ) override;
    void init(
        const std::string& option,
        const void *option2
    ) override;
    void done() override;
};

EXPORT_SHARED_C_FUNC AppBridge* makeFileJsonBridge();

#endif //TLNS_STDOUT_BRIDGE_H
