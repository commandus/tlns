#include "lorawan/lorawan-packet-storage.h"
#include "lorawan/lorawan-string.h"

LorawanPacketStorage::LorawanPacketStorage()
    : msg {}
{
}

std::string LorawanPacketStorage::toString() const
{
    return MHDR2String(this->msg.mhdr);
}

const DEVADDR* LorawanPacketStorage::getAddr() const
{
    if (msg.mhdr.f.mtype >= MTYPE_UNCONFIRMED_DATA_UP
        && msg.mhdr.f.mtype <= MTYPE_CONFIRMED_DATA_DOWN) {
        return &msg.data.downlink.devaddr;
    }
    return nullptr;
}

const JOIN_REQUEST_FRAME *LorawanPacketStorage::getJoinRequest() const
{
    if (msg.mhdr.f.mtype == MTYPE_JOIN_REQUEST)
        return &msg.data.joinRequest;
    return nullptr;
}
