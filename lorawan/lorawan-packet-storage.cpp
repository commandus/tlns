#include "lorawan-packet-storage.h"
#include "lorawan/lorawan-string.h"

LorawanPacketStorage::LorawanPacketStorage()
    : msg {}
{
}

std::string LorawanPacketStorage::toString() const
{
    return MHDR2String(this->msg.mhdr);
}
