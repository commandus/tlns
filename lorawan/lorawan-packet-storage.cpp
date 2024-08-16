#include <cstring>
#include <sstream>
#include <stdexcept>

#include "lorawan/lorawan-packet-storage.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/aes-helper.h"

#include "base64/base64.h"
#include "lorawan-conv.h"

LORAWAN_MESSAGE_STORAGE::LORAWAN_MESSAGE_STORAGE()
    : mhdr{}, data {}, packetSize(0)
{
}

LORAWAN_MESSAGE_STORAGE::LORAWAN_MESSAGE_STORAGE(
    const LORAWAN_MESSAGE_STORAGE& value
)
    : mhdr(value.mhdr), data{}, packetSize(value.packetSize)
{
    memmove(&data.u, &value.data.u, sizeof(data));
}

LORAWAN_MESSAGE_STORAGE::LORAWAN_MESSAGE_STORAGE(
    const std::string &base64string
)
    : mhdr{}, data {}, packetSize(0)
{
    decodeBase64ToLORAWAN_MESSAGE_STORAGE(*this, base64string);
}

void setLORAWAN_MESSAGE_STORAGE(
    LORAWAN_MESSAGE_STORAGE &retVal,
    const std::string &bin
)
{
    size_t sz = bin.size();
    memmove(&retVal.mhdr, bin.c_str(), sz);
    retVal.packetSize = (uint16_t) sz;
}

void setLORAWAN_MESSAGE_STORAGE(
    LORAWAN_MESSAGE_STORAGE &retVal,
    void *buffer,
    size_t size
)
{
    memmove(&retVal.mhdr, buffer, size);
    retVal.packetSize = (uint16_t) size;
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
    } catch (std::runtime_error &) {
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

/**
 * If buf parameter is NULL, calculate required buffer size.
 * @param buf Returning value can be NULL.
 * @param size buffer size. If buffer size is too small, check returning required size.
 * @param identity if provided, cipher data and add message integrity code (MIC)
 * @return required buffer size
 */
size_t LORAWAN_MESSAGE_STORAGE::toArray(
    void *buf,
    size_t size,
    const NetworkIdentity *identity
) const
{
    size_t retSize = 1;
    auto b = (uint8_t*) buf;
    if (b && (size >= retSize)) {
        *b = mhdr.i;
        b++;
    }
    switch ((MTYPE) mhdr.f.mtype) {
        case MTYPE_JOIN_REQUEST:
        case MTYPE_REJOIN_REQUEST:
            retSize += SIZE_JOIN_REQUEST_FRAME; // 18 bytes
            if (b && (size >= retSize)) {
                memmove(b, &data.joinRequest, SIZE_JOIN_REQUEST_FRAME);
            }
            break;
        case MTYPE_JOIN_ACCEPT:
            retSize += SIZE_JOIN_ACCEPT_FRAME;  // 16 bytes
            if (b && (size >= retSize)) {
                memmove(b, &data.u, SIZE_JOIN_ACCEPT_FRAME);
            }
            break;
        case MTYPE_UNCONFIRMED_DATA_UP:
        case MTYPE_CONFIRMED_DATA_UP:
            retSize += SIZE_UPLINK_EMPTY_STORAGE;  // 5 bytes
            if (b && (size >= retSize)) {
                memmove(b, &data.u, SIZE_UPLINK_EMPTY_STORAGE);
            }
            if (packetSize) {
                retSize += packetSize;
                if (b && (size >= retSize)) {
                    memmove(b, &data.uplink.optsNpayload, packetSize);
                }
            }
            break;
        case MTYPE_UNCONFIRMED_DATA_DOWN:
        case MTYPE_CONFIRMED_DATA_DOWN:
            retSize += SIZE_DOWNLINK_EMPTY_STORAGE;  // 5 bytes
            if (b && (size >= retSize)) {
                memmove(b, &data.u, SIZE_DOWNLINK_EMPTY_STORAGE);
            }
            if (packetSize) {
                retSize += packetSize;
                if (b && (size >= retSize)) {
                    memmove(b, &data.downlink.optsNpayload, packetSize);
                }
            }
            break;
        default:
            // case MTYPE_PROPRIETARYRADIO:
            break;
    }
    // apply host to network byte order
    applyNetworkByteOrder(buf, size);
    if (identity && packetSize > 0) {
        // cipher data
        switch ((MTYPE) mhdr.f.mtype) {
            case MTYPE_UNCONFIRMED_DATA_UP:
            case MTYPE_CONFIRMED_DATA_UP:
                encryptPayload((void *) &data.uplink.optsNpayload, (size_t) payloadSize,
                    data.uplink.fcnt, LORAWAN_UPLINK, identity->devaddr, identity->appSKey);
                break;
            case MTYPE_UNCONFIRMED_DATA_DOWN:
            case MTYPE_CONFIRMED_DATA_DOWN:
                encryptPayload((void *) &data.downlink.optsNpayload, (size_t) payloadSize,
                    data.downlink.fcnt, LORAWAN_UPLINK, identity->devaddr, identity->appSKey);
                break;
            default:
                break;
        }
    }
    // add MIC
    retSize += SIZE_MIC;
    if (b && (size >= retSize)) {
        uint32_t mic = calculateMIC(buf, size, *identity);
        memmove(b, &data.u, SIZE_MIC);
    }
    return retSize;
}

void LORAWAN_MESSAGE_STORAGE::decode(
    const NetworkIdentity *aIdentity
) {
    if (aIdentity)
        decode(aIdentity->devaddr, aIdentity->appSKey);
}

// decode message
void LORAWAN_MESSAGE_STORAGE::decode(
    const DEVADDR &devAddr,
    const KEY128 &appSKey
)
{
    // reapply network to host byte order
    applyHostByteOrder(&mhdr, sizeof(LORAWAN_MESSAGE_STORAGE));
    if (packetSize > 0) {
        switch (mhdr.f.mtype) {
            case MTYPE_JOIN_REQUEST:
            case MTYPE_REJOIN_REQUEST:
            case MTYPE_JOIN_ACCEPT:
                break;
            case MTYPE_UNCONFIRMED_DATA_UP:
            case MTYPE_CONFIRMED_DATA_UP:
                decryptPayload((void *) &data.uplink.optsNpayload, packetSize,
                    data.uplink.fcnt, LORAWAN_UPLINK, devAddr, appSKey);
            break;
            case MTYPE_UNCONFIRMED_DATA_DOWN:
            case MTYPE_CONFIRMED_DATA_DOWN:
                decryptPayload((void *) &data.downlink.optsNpayload, packetSize,
                    data.uplink.fcnt, LORAWAN_DOWNLINK, devAddr, appSKey);
                break;
            default:
                // case MTYPE_PROPRIETARYRADIO:
                break;
        }
    }
}

const DEVADDR* LORAWAN_MESSAGE_STORAGE::getAddr() const
{
    if (mhdr.f.mtype >= MTYPE_UNCONFIRMED_DATA_UP
        && mhdr.f.mtype <= MTYPE_CONFIRMED_DATA_DOWN) {
        return &data.downlink.devaddr;
    } else
        if (mhdr.f.mtype == MTYPE_JOIN_ACCEPT)
            return &data.joinResponse.hdr.devAddr;
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

LORAWAN_MESSAGE_STORAGE& LORAWAN_MESSAGE_STORAGE::operator=(
    const LORAWAN_MESSAGE_STORAGE &value
)
{
    mhdr = value.mhdr;
    memmove(&data.u, &value.data.u, sizeof(data));
    packetSize = value.packetSize;
    return *this;
}

std::string LORAWAN_MESSAGE_STORAGE::payloadString() const
{
    switch ((MTYPE) mhdr.f.mtype) {
        case MTYPE_UNCONFIRMED_DATA_UP:
        case MTYPE_CONFIRMED_DATA_UP:
            return std::string((char *) data.uplink.optsNpayload + data.uplink.f.foptslen,
                packetSize - data.uplink.f.foptslen);
        case MTYPE_UNCONFIRMED_DATA_DOWN:
        case MTYPE_CONFIRMED_DATA_DOWN:
            return std::string((char *) data.downlink.optsNpayload + data.downlink.f.foptslen,
                packetSize - data.downlink.f.foptslen);
        default:
            break;
    }
    return "";
}

std::string LORAWAN_MESSAGE_STORAGE::payloadBase64() const
{
    switch ((MTYPE) mhdr.f.mtype) {
        case MTYPE_UNCONFIRMED_DATA_UP:
        case MTYPE_CONFIRMED_DATA_UP:
            return base64_encode(std::string((char *) data.uplink.optsNpayload + data.uplink.f.foptslen,
                packetSize - data.uplink.f.foptslen));
        case MTYPE_UNCONFIRMED_DATA_DOWN:
        case MTYPE_CONFIRMED_DATA_DOWN:
            return base64_encode(std::string((char *) data.downlink.optsNpayload + data.downlink.f.foptslen,
                packetSize - data.downlink.f.foptslen));
        default:
            break;
    }
    return "";
}

bool UPLINK_STORAGE::operator==(
    const UPLINK_STORAGE &rhs
) const
{
    return memcmp(this, &rhs, SIZE_UPLINK_STORAGE) == 0;
}

bool DOWNLINK_STORAGE::operator==(
    const DOWNLINK_STORAGE &rhs
) const
{
    return memcmp(this, &rhs, SIZE_DOWNLINK_STORAGE) == 0;
}
