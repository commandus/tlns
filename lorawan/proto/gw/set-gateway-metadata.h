#ifndef SET_GATEWAY_METADATA_H
#define SET_GATEWAY_METADATA_H

#include "lorawan/proto/gw/gw.h"
#include "gateway-settings.h"

/**
 * Setup metadata for transmission gateway
 * @param retVal
 * @param gatewaySettings
 */
void setSEMTECH_PROTOCOL_METADATA_TX(
    SEMTECH_PROTOCOL_METADATA_TX &retVal,
    const GatewaySettings &gatewaySettings
);

#endif
