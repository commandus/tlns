#ifndef TLNS_STDOUT_BRIDGE_H
#define TLNS_STDOUT_BRIDGE_H

#include "lorawan/bridge/app-bridge.h"

class StdoutBridge : public AppBridge {
public:
    StdoutBridge() = default;
    virtual ~StdoutBridge() = default;
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

EXPORT_SHARED_C_FUNC AppBridge* makeBridge1();

#endif //TLNS_STDOUT_BRIDGE_H
