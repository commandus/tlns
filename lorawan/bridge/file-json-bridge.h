#ifndef TLNS_STDOUT_BRIDGE_H
#define TLNS_STDOUT_BRIDGE_H

#include "lorawan/bridge/app-bridge.h"

class FileJsonBridge : public AppBridge {
protected:
    std::string fileName;
    std::fstream *strm;
public:
    FileJsonBridge() = default;
    virtual ~FileJsonBridge() = default;
    void onPayload(
        const void* dispatcher,                 // MessageTaskDispatcher*
        const MessageQueueItem *messageItem     // network identity, gateway identifier and metadata etc.
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