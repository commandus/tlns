#include "lorawan/proto/gw/gw.h"

bool GwPushData::needConfirmation() const
{
    return rxData.mhdr.f.mtype == MTYPE_CONFIRMED_DATA_UP;
}

bool GwPullResp::needConfirmation() const
{
    return txData.mhdr.f.mtype == MTYPE_CONFIRMED_DATA_DOWN;
}
