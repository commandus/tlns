#include <cstring>
#include <sstream>
#include <stdexcept>

#include "lorawan/lorawan-packet-storage.h"
#include "lorawan/lorawan-string.h"

#include "base64/base64.h"

LORAWAN_MESSAGE_STORAGE::LORAWAN_MESSAGE_STORAGE()
    : data {}
{
}

LORAWAN_MESSAGE_STORAGE::LORAWAN_MESSAGE_STORAGE(
    const std::string &base64string
)
    : data {}
{
    decodeBase64ToLORAWAN_MESSAGE_STORAGE(*this, base64string);
}

bool decodeBase64ToLORAWAN_MESSAGE_STORAGE(
    LORAWAN_MESSAGE_STORAGE &retVal,
    const std::string &base64string
)
{
    try {
        std::string s = base64_decode(base64string);
        size_t sz = s.size();
        if (sz > sizeof(LORAWAN_MESSAGE_STORAGE) - 2)
            sz = sizeof(LORAWAN_MESSAGE_STORAGE) - 2;
        memmove(&retVal.mhdr, s.c_str(), sz);
        retVal.packetSize = (uint16_t) sz;
    } catch (std::runtime_error &e) {
        return false;
    }
    return true;
}

std::string LORAWAN_MESSAGE_STORAGE::toString() const
{
    std::stringstream ss;
    ss << "{\"mhdr\": " << MHDR2String(mhdr);
    switch ((MTYPE) mhdr.f.mtype) {
        case MTYPE_JOIN_REQUEST:
        case MTYPE_REJOIN_REQUEST:
            ss << ", \"joinRequest\": " << JOIN_REQUEST_FRAME2string(data.joinRequest);
            break;
        case MTYPE_JOIN_ACCEPT:
            ss << ", \"joinResponse\": " << JOIN_ACCEPT_FRAME2string(data.joinResponse);
            break;
        case MTYPE_UNCONFIRMED_DATA_UP:
        case MTYPE_CONFIRMED_DATA_UP:
            ss << ", \"uplink\": " << UPLINK_STORAGE2String(data.uplink, packetSize);
            break;
        case MTYPE_UNCONFIRMED_DATA_DOWN:
        case MTYPE_CONFIRMED_DATA_DOWN:
            ss << ", \"downlink\": " << DOWNLINK_STORAGE2String(data.downlink, packetSize);
            break;
        default:
            // case MTYPE_PROPRIETARYRADIO:
            break;
    }
    ss << "}";
    return ss.str();
}

const DEVADDR* LORAWAN_MESSAGE_STORAGE::getAddr() const
{
    if (mhdr.f.mtype >= MTYPE_UNCONFIRMED_DATA_UP
        && mhdr.f.mtype <= MTYPE_CONFIRMED_DATA_DOWN) {
        return &data.downlink.devaddr;
    }
    return nullptr;
}

const JOIN_REQUEST_FRAME *LORAWAN_MESSAGE_STORAGE::getJoinRequest() const
{
    if (mhdr.f.mtype == MTYPE_JOIN_REQUEST)
        return &data.joinRequest;
    return nullptr;
}

bool LORAWAN_MESSAGE_STORAGE::operator==(const LORAWAN_MESSAGE_STORAGE &rhs) const
{
    if (rhs.mhdr != mhdr)
        return false;
    switch(rhs.mhdr.f.mtype) {
        case MTYPE_JOIN_REQUEST:
            return rhs.data.joinRequest == data.joinRequest;
        case MTYPE_JOIN_ACCEPT:
            return rhs.data.joinResponse == data.joinResponse;
        case MTYPE_UNCONFIRMED_DATA_UP:
            return rhs.data.uplink == data.uplink;
        case MTYPE_UNCONFIRMED_DATA_DOWN:
            return rhs.data.downlink == data.downlink;
        case MTYPE_CONFIRMED_DATA_UP:
            return rhs.data.uplink == data.uplink;
        case MTYPE_CONFIRMED_DATA_DOWN:
            return rhs.data.downlink == data.downlink;
        case MTYPE_REJOIN_REQUEST:
            return false;
        case MTYPE_PROPRIETARYRADIO:
            return false;
        default:
            return false;
    }
}

bool UPLINK_STORAGE::operator==(
    const UPLINK_STORAGE &rhs
) const
{
    return memcmp(this, &rhs, SIZE_UPLINK_STORAGE);
}

bool DOWNLINK_STORAGE::operator==(
    const DOWNLINK_STORAGE &rhs
) const
{
    return memcmp(this, &rhs, SIZE_DOWNLINK_STORAGE);
}
