#include "lorawan/storage/client/device-best-gateway-direct-client.h"

DeviceBestGatewayDirectClient::DeviceBestGatewayDirectClient()
    : svc(nullptr)
{

}

DeviceBestGatewayDirectClient::~DeviceBestGatewayDirectClient() = default;

DeviceBestGatewayDirectClient::DeviceBestGatewayDirectClient(
    DeviceBestGatewayService* aSvc
)
    : svc(aSvc)
{

}