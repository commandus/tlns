#include <cstring>
#include <sstream>
#include <stdexcept>

#include "lorawan/lorawan-packet-storage.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/aes-helper.h"

#include "base64/base64.h"
#include "lorawan-conv.h"
#include "lorawan-mic.h"

LORAWAN_MESSAGE_STORAGE::LORAWAN_MESSAGE_STORAGE()
    : mhdr{}, data {}, payloadSize(0)
{
}

LORAWAN_MESSAGE_STORAGE::LORAWAN_MESSAGE_STORAGE(
    const LORAWAN_MESSAGE_STORAGE& value
)
    : mhdr(value.mhdr), data{}, payloadSize(value.payloadSize)
{
    memmove(&data.u, &value.data.u, sizeof(data));
}

LORAWAN_MESSAGE_STORAGE::LORAWAN_MESSAGE_STORAGE(
    const std::string &base64string
)
    : mhdr{}, data {}, payloadSize(0)
{
    base64SetToLORAWAN_MESSAGE_STORAGE(*this, base64string);
}

void setLORAWAN_MESSAGE_STORAGE(
    LORAWAN_MESSAGE_STORAGE &retVal,
    const std::string &bin
)
{
    size_t sz = bin.size();
    memmove(&retVal.mhdr, bin.c_str(), sz);
    retVal.payloadSize = (uint16_t) sz;
}

void setLORAWAN_MESSAGE_STORAGE(
    LORAWAN_MESSAGE_STORAGE &retVal,
    void *buffer,
    size_t size
)
{
    if (size > sizeof(LORAWAN_MESSAGE_STORAGE) - 2)
        size = sizeof(LORAWAN_MESSAGE_STORAGE) - 2;
    // including least 4 bytes MIC
    memmove(&retVal.mhdr, buffer, size);
    retVal.setSize(size);
}

bool base64SetToLORAWAN_MESSAGE_STORAGE(
    LORAWAN_MESSAGE_STORAGE &retVal,
    const std::string &base64string
)
{
    try {
        std::string s = base64_decode(base64string);
        setLORAWAN_MESSAGE_STORAGE(retVal, (void *) s.c_str(), s.size());
    } catch (std::runtime_error &) {
        return false;
    }
    return true;
}

bool hexSetToLORAWAN_MESSAGE_STORAGE(
    LORAWAN_MESSAGE_STORAGE &retVal,
    const std::string &hexString
)
{
    std::string s = hex2string(hexString);
    setLORAWAN_MESSAGE_STORAGE(retVal, (void *) s.c_str(), s.size());
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
            ss << ", \"uplink\": " << UPLINK_STORAGE2String(data.uplink, payloadSize);
            break;
        case MTYPE_UNCONFIRMED_DATA_DOWN:
        case MTYPE_CONFIRMED_DATA_DOWN:
            ss << ", \"downlink\": " << DOWNLINK_STORAGE2String(data.downlink, payloadSize);
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
            retSize += SIZE_UPLINK_EMPTY_STORAGE;  // 7 bytes
            if (b && (size >= retSize)) {
                memmove(b, &data.uplink.devaddr.c, SIZE_UPLINK_EMPTY_STORAGE);
                b += SIZE_UPLINK_EMPTY_STORAGE;
            }
            if (payloadSize) {
                retSize += payloadSize + 1; // + FPort
                if (b && (size >= retSize)) {
                    *b = data.uplink.fport();
                    b++;
                    memmove(b, data.uplink.payload(), payloadSize);
                    b += payloadSize;
                }
            }
            break;
        case MTYPE_UNCONFIRMED_DATA_DOWN:
        case MTYPE_CONFIRMED_DATA_DOWN:
            retSize += SIZE_DOWNLINK_EMPTY_STORAGE;  // 7 bytes
            if (b && (size >= retSize)) {
                memmove(b, &data.downlink.devaddr.c, SIZE_DOWNLINK_EMPTY_STORAGE);
                b += SIZE_DOWNLINK_EMPTY_STORAGE;
            }
            if (payloadSize) {
                retSize += payloadSize + 1; // + FPort
                if (b && (size >= retSize)) {
                    *b = data.uplink.fport();
                    b++;
                    memmove(b, data.downlink.payload(), payloadSize);
                    b += payloadSize;
                }
            }
            break;
        default:
            // case MTYPE_PROPRIETARYRADIO:
            break;
    }
    // apply host to network byte order
    applyNetworkByteOrder(buf, size);
    if (identity && payloadSize > 0) {
        // cipher data
        switch ((MTYPE) mhdr.f.mtype) {
            case MTYPE_UNCONFIRMED_DATA_UP:
            case MTYPE_CONFIRMED_DATA_UP:
                encryptPayload((void *) data.uplink.payload(), (size_t) payloadSize,
                    data.uplink.fcnt, LORAWAN_UPLINK, identity->devaddr, identity->appSKey);
                break;
            case MTYPE_UNCONFIRMED_DATA_DOWN:
            case MTYPE_CONFIRMED_DATA_DOWN:
                encryptPayload((void *) data.downlink.payload(), (size_t) payloadSize,
                    data.downlink.fcnt, LORAWAN_UPLINK, identity->devaddr, identity->appSKey);
                break;
            default:
                break;
        }
    }
    // add MIC
    retSize += SIZE_MIC;
    if (identity && b && (size >= retSize)) {
        uint32_t mic = calculateMIC(buf, size, *identity);
        mic = NTOH4(mic);
        memmove(b, &mic, SIZE_MIC);
    }
    return retSize;
}

size_t LORAWAN_MESSAGE_STORAGE::toStream(
    std::ostream &retVal,
    const NetworkIdentity *aIdentity
) const
{
    char b[300];
    auto r = toArray((void *) b, sizeof(b), aIdentity);
    retVal.write(b, (std::streamsize) r);
    return r;
}

std::string LORAWAN_MESSAGE_STORAGE::asHex(
    const NetworkIdentity *aIdentity
) const
{
    char b[300];
    size_t r = toArray((void *) b, sizeof(b), aIdentity);
    return hexString(b, r);
}

bool LORAWAN_MESSAGE_STORAGE::decode(
    const NetworkIdentity *aIdentity
) {
    if (aIdentity) {
        return decode(aIdentity->devaddr, aIdentity->appSKey);
    }
    return false;
}

// decode message
bool LORAWAN_MESSAGE_STORAGE::decode(
    const DEVADDR &devAddr,
    const KEY128 &appSKey
)
{
    // reapply network to host byte order
    applyHostByteOrder(&mhdr, sizeof(LORAWAN_MESSAGE_STORAGE));
    if (payloadSize > 0) {
        switch (mhdr.f.mtype) {
            case MTYPE_JOIN_REQUEST:
            case MTYPE_REJOIN_REQUEST:
            case MTYPE_JOIN_ACCEPT:
                break;
            case MTYPE_UNCONFIRMED_DATA_UP:
            case MTYPE_CONFIRMED_DATA_UP:
                decryptPayload((void *) data.uplink.payload(), payloadSize,
                    data.uplink.fcnt, LORAWAN_UPLINK, devAddr, appSKey);
                return true;
            case MTYPE_UNCONFIRMED_DATA_DOWN:
            case MTYPE_CONFIRMED_DATA_DOWN:
                decryptPayload((void *) data.downlink.payload(), payloadSize,
                    data.uplink.fcnt, LORAWAN_DOWNLINK, devAddr, appSKey);
                return true;
            default:
                // case MTYPE_PROPRIETARYRADIO:
                break;
        }
    }
    return false;
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
    payloadSize = value.payloadSize;
    return *this;
}

std::string LORAWAN_MESSAGE_STORAGE::payloadString() const
{
    switch ((MTYPE) mhdr.f.mtype) {
        case MTYPE_UNCONFIRMED_DATA_UP:
        case MTYPE_CONFIRMED_DATA_UP:
            return std::string((const char *) data.uplink.payload(), payloadSize);
        case MTYPE_UNCONFIRMED_DATA_DOWN:
        case MTYPE_CONFIRMED_DATA_DOWN:
            return std::string((const char *) data.downlink.payload(), payloadSize);
        default:
            break;
    }
    return "";
}

void LORAWAN_MESSAGE_STORAGE::setSize(
    size_t size
) {
    switch (mhdr.f.mtype) {
        case MTYPE_JOIN_REQUEST:
        case MTYPE_REJOIN_REQUEST:
        case MTYPE_JOIN_ACCEPT:
            break;
        case MTYPE_UNCONFIRMED_DATA_UP:
        case MTYPE_CONFIRMED_DATA_UP:
            payloadSize = size - SIZE_MHDR - SIZE_UPLINK_EMPTY_STORAGE - data.uplink.f.foptslen - SIZE_MIC;
            // FPort
            if (payloadSize > 1)
                payloadSize--;
            if (payloadSize < 0)
                payloadSize = 0;
            break;
        case MTYPE_UNCONFIRMED_DATA_DOWN:
        case MTYPE_CONFIRMED_DATA_DOWN:
            payloadSize = size - SIZE_MHDR - SIZE_DOWNLINK_EMPTY_STORAGE - data.downlink.f.foptslen - SIZE_MIC;
            // FPort
            if (payloadSize > 1)
                payloadSize--;
            if (payloadSize < 0)
                payloadSize = 0;
            break;
        default:
            break;
    }
}

uint32_t LORAWAN_MESSAGE_STORAGE::mic(
    const KEY128 &key
) const
{
    switch ((MTYPE) mhdr.f.mtype) {
        case MTYPE_UNCONFIRMED_DATA_UP:
        case MTYPE_CONFIRMED_DATA_UP:
        case MTYPE_UNCONFIRMED_DATA_DOWN:
        case MTYPE_CONFIRMED_DATA_DOWN:
            return calculateMICFrmPayload(&mhdr.i, payloadSize ? payloadSize + 9 : 8, data.uplink.fcnt, mhdr.f.mtype & 1, data.uplink.devaddr, key);
        default:
            break;
    }
    return 0;
}

bool LORAWAN_MESSAGE_STORAGE::matchMic(
    const KEY128 &key
) const {
    return mic() == mic(key);
}

/**
 * MIC saved in the buffer at the end of payload in the Semtech's simple UDP protocol as part of radio packet
 * @return 0 if there os no room for MIC (in case of simulation wire protocol)
 */
uint32_t LORAWAN_MESSAGE_STORAGE::mic() const
{
    switch ((MTYPE) mhdr.f.mtype) {
        case MTYPE_UNCONFIRMED_DATA_UP:
        case MTYPE_CONFIRMED_DATA_UP:
        case MTYPE_UNCONFIRMED_DATA_DOWN:
        case MTYPE_CONFIRMED_DATA_DOWN:
            if (payloadSize <= 255 - 4)
                return *(uint32_t*) (&mhdr.i + 1 + SIZE_DOWNLINK_EMPTY_STORAGE + (payloadSize ? 1 + payloadSize : 0));
            else
                return 0;
        default:
            break;
    }
    return 0;
}

const std::string LORAWAN_MESSAGE_STORAGE::foptsString() const {
    switch ((MTYPE) mhdr.f.mtype) {
        case MTYPE_UNCONFIRMED_DATA_UP:
        case MTYPE_CONFIRMED_DATA_UP:
            return data.uplink.foptsString();
        case MTYPE_UNCONFIRMED_DATA_DOWN:
        case MTYPE_CONFIRMED_DATA_DOWN:
            return data.downlink.foptsString();
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
            return base64_encode(std::string((char *) data.uplink.payload(), payloadSize));
        case MTYPE_UNCONFIRMED_DATA_DOWN:
        case MTYPE_CONFIRMED_DATA_DOWN:
            return base64_encode(std::string((char *) data.downlink.payload(), payloadSize));
        default:
            break;
    }
    return "";
}

void LORAWAN_MESSAGE_STORAGE::setPayload(
    const void* value,
    size_t size
)
{
    switch ((MTYPE) mhdr.f.mtype) {
        case MTYPE_UNCONFIRMED_DATA_UP:
        case MTYPE_CONFIRMED_DATA_UP:
            data.uplink.setPayload((uint8_t*) value, (uint8_t) size);
        case MTYPE_UNCONFIRMED_DATA_DOWN:
        case MTYPE_CONFIRMED_DATA_DOWN:
            data.downlink.setPayload((uint8_t*) value, (uint8_t) size);
        default:
            break;
    }
}

void LORAWAN_MESSAGE_STORAGE::setFOpts(
    const void* value,
    size_t size
)
{
    switch ((MTYPE) mhdr.f.mtype) {
        case MTYPE_UNCONFIRMED_DATA_UP:
        case MTYPE_CONFIRMED_DATA_UP:
            data.uplink.setFopts((uint8_t*) value, (uint8_t) size);
        case MTYPE_UNCONFIRMED_DATA_DOWN:
        case MTYPE_CONFIRMED_DATA_DOWN:
            data.downlink.setFopts((uint8_t*) value, (uint8_t) size);
        default:
            break;
    }
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

const uint8_t* DOWNLINK_STORAGE::fopts() const
{
    return fopts_fport_payload;
}

const std::string DOWNLINK_STORAGE::foptsString() const {
    return std::string((const char *) fopts_fport_payload, f.foptslen);
}

uint8_t DOWNLINK_STORAGE::foptsSize() const
{
    return f.foptslen;
}

void DOWNLINK_STORAGE::setFopts(
    uint8_t* value,
    uint8_t size
)
{
    f.foptslen = size & 0xf;
    memmove(fopts_fport_payload, value, f.foptslen);
}

uint8_t DOWNLINK_STORAGE::fport() const
{
    return fopts_fport_payload[f.foptslen];
}

void DOWNLINK_STORAGE::setFport(
    uint8_t value
)
{
    fopts_fport_payload[f.foptslen] = value;
}

const uint8_t* DOWNLINK_STORAGE::payload() const
{
    return &fopts_fport_payload[f.foptslen + 1];
}

void DOWNLINK_STORAGE::setPayload(
    const void* value,
    uint8_t size
) {
    memmove(fopts_fport_payload + f.foptslen + 1, value, size);
}

void DOWNLINK_STORAGE::setFOpts(
    const void* value,
    size_t size
) {
    // check size
    if (size > 16)
        size = 16;
    // make window to insert: move FPort + payload
    memmove(fopts_fport_payload + size, fopts_fport_payload, FOPTS_FPORT_PAYLOAD_SIZE - size);
    // set FOpts
    memmove(fopts_fport_payload, value, size);
}

const uint8_t* UPLINK_STORAGE::fopts() const
{
    return fopts_fport_payload;
}

const std::string UPLINK_STORAGE::foptsString() const {
    return std::string((const char *) fopts_fport_payload, f.foptslen);
}

uint8_t UPLINK_STORAGE::foptsSize() const
{
    return f.foptslen;
}

void UPLINK_STORAGE::setFopts(
    const uint8_t* value,
    uint8_t size
)
{
    f.foptslen = size & 0xf;
    memmove(fopts_fport_payload, value, f.foptslen);
}

uint8_t UPLINK_STORAGE::fport() const
{
    return fopts_fport_payload[f.foptslen];
}

void UPLINK_STORAGE::setFport(
    uint8_t value
)
{
    fopts_fport_payload[f.foptslen] = value;
}

const uint8_t* UPLINK_STORAGE::payload() const
{
    return &fopts_fport_payload[f.foptslen + 1];
}

void UPLINK_STORAGE::setPayload(
    const void *value,
    uint8_t size
) {
    memmove(fopts_fport_payload + f.foptslen + 1, value, size); // +1 FPort
}

void UPLINK_STORAGE::setFOpts(
    const void* value,
    size_t size
) {
    // check size
    if (size > 16)
        size = 16;
    // make window to insert: move FPort + payload
    memmove(fopts_fport_payload + size, fopts_fport_payload, FOPTS_FPORT_PAYLOAD_SIZE - size);
    // set FOpts
    memmove(fopts_fport_payload, value, size);
}
