#ifndef SET_GATEWAY_METADATA_H
#define SET_GATEWAY_METADATA_H

#include "lorawan/proto/gw/gw.h"
#include "lorawan/regional-parameters/regional-parameter-channel-plan.h"

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
    const RegionalParameterChannelPlan &plan,
    const SEMTECH_PROTOCOL_METADATA_RX &rxMetadata,
    uint16_t payloadSize
);

#endif
