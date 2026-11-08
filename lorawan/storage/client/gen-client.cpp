#include "gen-client.h"

#include "lorawan/lorawan-string.h"

#include "lorawan/storage/service/identity-service-gen.h"
#include "lorawan/storage/service/gateway-service-json.h"
#ifdef ENABLE_DEBUG
#include <iostream>
#include <cstring>
#include "lorawan-msg.h"
#endif

GenClient::GenClient(
    const std::string &masterKey
)
	: DirectClient()
{
    svcIdentity = new GenIdentityService(masterKey);
    svcGateway = new MemoryGatewayService;
}

GenClient::GenClient(
    const std::string &masterKey,
    const GatewayIdentity &gateway
)
    : GenClient(masterKey)
{
    svcGateway->put(gateway);
}

GenClient::~GenClient()
{
    if (svcIdentity)
        delete svcIdentity;
    if (svcGateway)
        delete svcGateway;
}
