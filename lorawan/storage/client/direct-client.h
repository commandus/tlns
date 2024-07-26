#ifndef DIRECT_CLIENT_H_
#define DIRECT_CLIENT_H_	1

#include "lorawan/storage/service/identity-service.h"
#include "lorawan/storage/service/gateway-service.h"

/**
 * Abstract class to load specific identity and gateway services
 * @see ServiceClient
 * @see PluginClient
 */
class DirectClient {
public:
    IdentityService* svcIdentity;
    GatewayService* svcGateway;
    DirectClient();
    virtual ~DirectClient();
};

#endif
