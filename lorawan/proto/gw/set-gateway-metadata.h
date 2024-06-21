#ifndef SET_GATEWAY_METADATA_H
#define SET_GATEWAY_METADATA_H

#include "lorawan/proto/gw/gw.h"
#include "gateway-settings.h"

/**
 * Setup metadata for transmission gateway.
 * Copy frequency, SF, DR from metadata of received packet.
 * @param retVal
 * @param gatewaySettings gateway regional settings
 * @param rxMetadata received message metadata including receiving time, frequency, RSSI etc.
 * @param payloadSize payload size in bytes
 */
void setSEMTECH_PROTOCOL_METADATA_TX(
    SEMTECH_PROTOCOL_METADATA_TX &retVal,
    const GatewaySettings &gatewaySettings,
    const SEMTECH_PROTOCOL_METADATA_RX &rxMetadata,
    uint16_t payloadSize
);

void invalidateSEMTECH_PROTOCOL_METADATA_TX(
    SEMTECH_PROTOCOL_METADATA_TX &val,
    const GatewaySettings &gatewaySettings
);

#endif
