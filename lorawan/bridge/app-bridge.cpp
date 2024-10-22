#include "lorawan/bridge/app-bridge.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/task/message-task-dispatcher.h"
#include "lorawan/lorawan-builder.h"

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

int AppBridge::send(
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
    return ((MessageTaskDispatcher *)dispatcher)->sendDownlink(tim, addr, payload, fopts, fPort,
        payloadSize, foptsSize, ((MessageTaskDispatcher *)dispatcher)->parsers[proto]);
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
    return send(tim, addr, nullptr, fopts, FPORT_NO_PAYLOAD, 0, size, proto);
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
    return send(tim, addr, payload, nullptr, fPort, size, 0, proto);
}

int AppBridge::send(
    const TASK_TIME &tim,
    const DEVADDR &addr,
    uint8_t fPort,
    const std::string &payload,
    const std::string &fOpts,
    uint8_t proto
)
{
    return send(tim, addr, (void *) payload.c_str(), (void *) fOpts.c_str(),
        fPort, payload.size(), fOpts.size(), proto);
}

int AppBridge::sendFOpts(
    const TASK_TIME &tim,
    const DEVADDR &addr,
    const std::string &fOpts,
    uint8_t proto
)
{
    return send(tim, addr, nullptr, (void *) fOpts.c_str(),
                FPORT_NO_PAYLOAD, 0, fOpts.size(), proto);
}

int AppBridge::sendPayload(
    const TASK_TIME &tim,
    const DEVADDR &addr,
    uint8_t fPort,
    const std::string &payload,
    uint8_t proto
)
{
    return send(tim, addr, (void *) payload.c_str(), nullptr,
        fPort, payload.size(), 0, proto);
}
