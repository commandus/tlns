#include "lorawan/bridge/app-bridge.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/task/message-task-dispatcher.h"

AppBridge::AppBridge()
    : dispatcher(nullptr)
{

}

void AppBridge::setDispatcher(
    void *aDispatcher
)
{
    dispatcher = aDispatcher;
}

int AppBridge::send2addr(
    const TASK_TIME &tim,
    const DEVADDR &addr,
    void *payload,
    void *fopts,
    uint8_t fPort,
    uint8_t payloadSize,
    uint8_t foptsSize,
    uint8_t proto
)
{
    if (!dispatcher)
        return ERR_CODE_WRONG_PARAM;
    if (proto >= ((MessageTaskDispatcher *)dispatcher)->parsers.size())
        return ERR_CODE_WRONG_PARAM;
    return ((MessageTaskDispatcher *) dispatcher)->enqueueDownlink(tim, addr, payload, fopts, fPort,
        payloadSize, foptsSize, ((MessageTaskDispatcher *) dispatcher)->parsers[proto]);
}

// wrappers

int AppBridge::sendFOpts(
    const TASK_TIME &tim,
    const DEVADDR &addr,
    void *fopts,
    uint8_t size,
    uint8_t proto
)
{
    return send2addr(tim, addr, nullptr, fopts, FPORT_NO_PAYLOAD, 0, size, proto);
}

int AppBridge::sendPayload(
    const TASK_TIME &tim,
    const DEVADDR &addr,
    uint8_t fPort,
    void *payload,
    uint8_t size,
    uint8_t proto
)
{
    return send2addr(tim, addr, payload, nullptr, fPort, size, 0, proto);
}

int AppBridge::sendString(
    const TASK_TIME &tim,
    const DEVADDR &addr,
    uint8_t fPort,
    const std::string &payload,
    const std::string &fOpts,
    uint8_t proto
)
{
    return send2addr(tim, addr, (void *) payload.c_str(), (void *) fOpts.c_str(),
        fPort, (uint8_t) payload.size(), (uint8_t) fOpts.size(), proto);
}

int AppBridge::sendFOpts(
    const TASK_TIME &tim,
    const DEVADDR &addr,
    const std::string &fOpts,
    uint8_t proto
)
{
    return send2addr(tim, addr, nullptr, (void *) fOpts.c_str(),
        FPORT_NO_PAYLOAD, 0, (uint8_t) fOpts.size(), proto);
}

int AppBridge::sendPayload(
    const TASK_TIME &tim,
    const DEVADDR &addr,
    uint8_t fPort,
    const std::string &payload,
    uint8_t proto
)
{
    return send2addr(tim, addr, (void *) payload.c_str(), nullptr,
        fPort, (uint8_t ) payload.size(), 0, proto);
}
