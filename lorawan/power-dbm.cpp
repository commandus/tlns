#include "lorawan/power-dbm.h"

int gwPower(
    const SEMTECH_PROTOCOL_METADATA_RX *rxMetadata,
    const RegionalParameterChannelPlan *pPlan
)
{
    if (pPlan)
        return pPlan->get()->defaultDownlinkTXPower;
    return 14;  // dBm
}

int gwPowerTx(
    const SEMTECH_PROTOCOL_METADATA_TX *txMetadata,
    const RegionalParameterChannelPlan *pPlan
) {
    if (pPlan)
        return pPlan->get()->defaultDownlinkTXPower;
    return 14;  // dBm
}
