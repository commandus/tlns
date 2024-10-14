#ifndef TLNS_POWER_DBM_H
#define TLNS_POWER_DBM_H

#include "lorawan-types.h"
#include "lorawan/regional-parameters/regional-parameter-channel-plan.h"

/**
 * Calculate power in dBm
 * @param rxMetadata received message metadata: RSSI, Lora SNR ratio etc.
 * @param regionalPlan regional parameters: max Uplink EIRP etc.
 * @return 14
 */
int gwPower(
    const SEMTECH_PROTOCOL_METADATA_RX *rxMetadata,
    const RegionalParameterChannelPlan *regionalPlan
);

#endif
