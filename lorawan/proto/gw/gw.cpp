#include "lorawan/proto/gw/gw.h"

bool GwPushData::needConfirmation()
{
    return rxData.mhdr.f.mtype == MTYPE_CONFIRMED_DATA_UP;
}

bool GwPullResp::needConfirmation()
{
    return txData.mhdr.f.mtype == MTYPE_CONFIRMED_DATA_DOWN;
}
