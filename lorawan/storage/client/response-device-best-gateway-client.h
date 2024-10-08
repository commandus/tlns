#ifndef RESPONSE_DEVICE_BEST_GATEWAY_CLIENT_H_
#define RESPONSE_DEVICE_BEST_GATEWAY_CLIENT_H_

#include "lorawan/lorawan-types.h"

/**
 * Asynchronous response abstract class for device's best gateway client/service
 */
class DeviceBestGatewayResponseClient {
public:
    virtual void onDeviceBestGatewayGet(
        DeviceBestGatewayResponseClient* client,
        const DEVADDR &devAddr,
        uint64_t gatewayId
    ) = 0;
    virtual void onDeviceBestGatewayOperation(
        DeviceBestGatewayResponseClient* client,
        const DEVADDR &devAddr,
        uint32_t code
    ) = 0;
    virtual void onDeviceBestGatewayList(
        DeviceBestGatewayResponseClient* client,
        std::map<DEVADDR, uint64_t> &response
    ) = 0;
    virtual void onDeviceBestGatewaySize(
        DeviceBestGatewayResponseClient* client,
        size_t response
    ) = 0;
};

#endif
