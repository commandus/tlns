#include "lorawan/proto/gw/gw.h"

bool GwPushData::needConfirmation() const
{
    return rxData.mhdr.f.mtype == MTYPE_CONFIRMED_DATA_UP;
}

bool GwPushData::isMetadataValid() const
{
    return rxMetadata.modu == MODULATION_LORA || rxMetadata.modu == MODULATION_FSK;
}

bool GwPullResp::needConfirmation() const
{
    return txData.mhdr.f.mtype == MTYPE_CONFIRMED_DATA_DOWN;
}

bool GwPullResp::isMetadataValid() const
{
    return txMetadata.modulation == MODULATION_LORA || txMetadata.modulation == MODULATION_FSK;
}
