#ifndef DEVICE_BEST_GATEWAY_DIRECT_CLIENT_H_
#define DEVICE_BEST_GATEWAY_DIRECT_CLIENT_H_	1

#include "lorawan/storage/service/device-best-gateway.h"

/**
 * Class to access best gateway service
 * @see DeviceBestGatewayService
 */
class DeviceBestGatewayDirectClient {
public:
    DeviceBestGatewayService* svc;
    DeviceBestGatewayDirectClient();
    DeviceBestGatewayDirectClient(DeviceBestGatewayService* svc);
    virtual ~DeviceBestGatewayDirectClient();
};

#endif
