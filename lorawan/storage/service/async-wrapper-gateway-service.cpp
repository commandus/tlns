#include "async-wrapper-gateway-service.h"

AsyncWrapperGatewayService::AsyncWrapperGatewayService() = default;

void AsyncWrapperGatewayService::get(
    const GatewayIdentity &request,
    const std::function<void(
        int retCode,
        GatewayIdentity &retVal
    )>& cb
)
{
    GatewayIdentity gatewayIdentity;
    int r = gatewayService->get(gatewayIdentity, request);
    cb(r, gatewayIdentity);
}

// Add or replace Address = EUI and keys pair
void AsyncWrapperGatewayService::put(
    const GatewayIdentity &identity,
    const std::function<void(
        int retCode
    )>& cb
)
{
    int r = gatewayService->put(identity);
    cb(r);
}

// Remove entry
void AsyncWrapperGatewayService::rm(
    const GatewayIdentity &identity,
    const std::function<void(
        int retCode
    )>& cb
)
{
    int r = gatewayService->rm(identity);
    cb(r);
}

// List entries
void AsyncWrapperGatewayService::list(
    uint32_t offset,
    uint8_t size,
    const std::function<void(
        int retCode,
        std::vector<GatewayIdentity> &retVal
    )>& cb
)
{
    std::vector<GatewayIdentity> gatewayIdentities;
    int r = gatewayService->list(gatewayIdentities, offset, size);
    cb(r, gatewayIdentities);
}

// Entries count
void AsyncWrapperGatewayService::size(
    const std::function<void(
        size_t size
   )>& cb
)
{
    cb(gatewayService->size());
}

// force save
void AsyncWrapperGatewayService::flush(
    const std::function<void(
        int retCode
    )>& cb
)
{
    gatewayService->flush();
    cb(0);
}

// reload
void AsyncWrapperGatewayService::init(
    const std::string &option,
    void *data,
    const std::function<void(
        int retCode
    )>& cb
)
{
    cb(gatewayService->init(option, data));
}

// close resources
void AsyncWrapperGatewayService::done(
    const std::function<void(
        int retCode
    )>& cb
)
{
    gatewayService->done();
    cb(0);
}

void AsyncWrapperGatewayService::setOption(
    int option,
    void *value,
    const std::function<void(
        int retCode
    )>& cb
)
{
    gatewayService->setOption(option, value);
    cb(0);
}