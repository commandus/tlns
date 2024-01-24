#include <cstring>
#include <stdexcept>

#include "lorawan/lorawan-packet-storage.h"
#include "lorawan/lorawan-string.h"

#include "base64/base64.h"

LorawanPacketStorage::LorawanPacketStorage()
    : msg {}
{
}

LorawanPacketStorage::LorawanPacketStorage(
    const std::string &base64string
)
    : msg {}
{
    decodeBase64ToLORAWAN_MESSAGE_STORAGE(msg, base64string);
}

bool decodeBase64ToLORAWAN_MESSAGE_STORAGE(
    LORAWAN_MESSAGE_STORAGE &retVal,
    const std::string &base64string
)
{
    try {
        std::string s = base64_decode(base64string);
        size_t sz = s.size();
        if (sz > sizeof(LORAWAN_MESSAGE_STORAGE))
            sz = sizeof(LORAWAN_MESSAGE_STORAGE);
        memmove(&retVal.mhdr, s.c_str(), sz);
    } catch (std::runtime_error e) {
        return false;
    }
    return true;
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
