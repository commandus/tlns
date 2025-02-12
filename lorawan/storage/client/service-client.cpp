#include "service-client.h"

#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-conv.h"

#include "lorawan/storage/service/identity-service-json.h"
#include "lorawan/storage/service/identity-service-gen.h"
#include "lorawan/storage/service/gateway-service-json.h"
#ifdef ENABLE_SQLITE
#include "lorawan/storage/service/identity-service-sqlite.h"
#include "lorawan/storage/service/gateway-service-sqlite.h"
#endif

#ifdef ENABLE_DEBUG
#include <iostream>
#include <cstring>
#include "lorawan-msg.h"
#endif

ServiceClient::ServiceClient(
    const std::string &name
)
	: DirectClient()
{
    if (name == "json") {
        svcIdentity = new JsonIdentityService;
        svcGateway = new JsonGatewayService;
    } else {
        if (name == "gen") {
            svcIdentity = new GenIdentityService;
            svcGateway = new MemoryGatewayService;
        } else {
            if (name == "mem") {
                svcIdentity = new MemoryIdentityService;
                svcGateway = new MemoryGatewayService;
            } else {
#ifdef ENABLE_SQLITE
                if (name == "sqlite") {
                    svcIdentity = new SqliteIdentityService;
                    svcGateway = new SqliteGatewayService;
                }
#endif
            }
        }
    }
}

bool ServiceClient::hasStaticPlugin(
    const std::string &name
)
{
    if (name == "json" || name == "gen" || name == "mem")
        return true;
    else {
#ifdef ENABLE_SQLITE
        if (name == "sqlite")
            return true;
#endif
    }
    return false;
}

ServiceClient::~ServiceClient()
{
    if (svcIdentity)
        delete svcIdentity;
    if (svcGateway)
        delete svcGateway;
}
