#ifndef ASYNC_WRAPPER_GATEWAY_SERVICE_H_
#define ASYNC_WRAPPER_GATEWAY_SERVICE_H_ 1

#include <functional>
#include "gateway-service.h"

/**
 * Identity service interface
 * Get device identifier and keys by the network address
 */ 
class AsyncWrapperGatewayService {
private:
    GatewayService *gatewayService;
public:
    AsyncWrapperGatewayService();
    void get(
        const GatewayIdentity &request,
        const std::function<void(
            int retCode,
            GatewayIdentity &retVal
        )>& cb
    );

    // Add or replace Address = EUI and keys pair
    void put(
        const GatewayIdentity &identity,
        const std::function<void(
            int retCode
        )>& cb
    );

    // Remove entry
    void rm(
        const GatewayIdentity &identity,
        const std::function<void(
            int retCode
        )>& cb
    );

    // List entries
    void list(
        uint32_t offset,
        uint8_t size,
        const std::function<void(
            int retCode,
            std::vector<GatewayIdentity> &retVal
        )>& cb
    );

    // Entries count
    void size(
        const std::function<void(
            size_t size
        )>& cb
    );

    // force save
    void flush(
        const std::function<void(
            int retCode
        )>& cb
    );

    // reload
    void init(
        const std::string &option,
        void *data,
        const std::function<void(
            int retCode
        )>& cb
    );

    // close resources
    void done(
        const std::function<void(
            int retCode
        )>& cb
    );

    void setOption(
        int option,
        void *value,
        const std::function<void(
            int retCode
        )>& cb
    );

};

#endif
