#include <cstring>
#include <sstream>
#include "lorawan-types.h"
#include "lorawan-string.h"
#include "lorawan-conv.h"
#include "lorawan-error.h"

const char *LIST_SEPARATOR = ",";

#if defined(_MSC_VER) || defined(__MINGW32__)
#pragma warning(disable: 4996)
#endif

LORAWAN_VERSION::LORAWAN_VERSION()
    : c(1)
{
    // major = 1;
    // minor = 0;
    // release = 0;
}

LORAWAN_VERSION::LORAWAN_VERSION(
    uint8_t amajor,
    uint8_t aminor,
    uint8_t arelease
)
{
    major = amajor;
    minor = aminor;
    release = arelease;
}

LORAWAN_VERSION::LORAWAN_VERSION(
    uint8_t value
)
{
    c = value;
}

// <------------------- NETID -------------------

NETID::NETID()
{
    memset(&c, 0, sizeof(NETID));
}

NETID::NETID(
	const NETID &value
) {
    memmove(&c, &value.c, sizeof(NETID));
}

NETID::NETID(
    uint32_t value
)
{
    int2NETID(*this, value);
}

NETID::NETID(
    uint8_t netType,
    uint32_t value
)
{
    set(netType, value);
}

uint8_t NETID::getType() const
{
    return ((NETID_TYPE*) &c)->networkType;
}

void NETID::get(
	NETID &retval
) const
{
    memmove(&retval.c, &c, sizeof(NETID));
}

NETID *NETID::getPtr() const
{
    return (NETID *) &c;
}

std::string NETID::toString() const
{
    std::stringstream ss;
    ss << std::hex << NETID2int(*this);
    return ss.str();
}

uint32_t NETID::get() const
{
    return NETID2int(*this);
}

static uint32_t MASK_NWK_ID[8] = {
    (1 << 6) - 1,
    (1 << 6) - 1,
    (1 << 9) - 1,
    (1 << 10) - 1,
    (1 << 11) - 1,
    (1 << 13) - 1,
    (1 << 15) - 1,
    (1 << 17) - 1
};

uint32_t NETID::getNetId() const
{
    return NETID2int(*this) & getTypeMask();
}

uint32_t NETID::getNwkId() const
{
    return NETID2int(*this) & MASK_NWK_ID[((NETID_TYPE*) this)->networkType];
}

void NETID::setType(
	uint8_t value
)
{
    ((NETID_TYPE*) this)->networkType = value;
}

void NETID::set(
	const NETID &value
) {
    memmove(this->c, &value, sizeof(NETID));
}

void NETID::set(
	uint32_t value
) {
    int2NETID(*this, value);
}

void NETID::set(
	const std::string &value
)
{
    std::stringstream ss(value);
    uint32_t r;
    ss >> std::hex >> r;
    set(r);
}

int NETID::set(
    uint8_t netType,
    uint32_t value
)
{
    if (netType > 7)
        return ERR_CODE_NETTYPE_OUT_OF_RANGE;
    switch (netType) {
        case 0:
        case 1:
            if (value >= (1 << 6))
                return ERR_CODE_NETID_OUT_OF_RANGE;
            break;
        case 2:
            if (value >= (1 << 9))
                return ERR_CODE_NETID_OUT_OF_RANGE;
            break;
        default:    // 3..7
            if (value >= (1 << 21))
                return ERR_CODE_NETID_OUT_OF_RANGE;
            break;
    }
    int2NETID(*this, value);
    setType(netType);
    return 0;
}

/**
 * Invalidate NetId, set RFU to zeroes
 */
void NETID::applyTypeMask()
{
    set((uint32_t) get() & getTypeMask());
}

/**
 * NETID has 8 types
 * @see LoRaWAN Backend Interfaces 1.0 Specification
 *      Chapter 13 DevAddr Assignment
 * @return
 */
int NETID::getTypeMask() const
{
    switch (((NETID_TYPE*) this)->networkType) {
        case 0:
        case 1:
            return (1 << 6) - 1;    // 15 unused bits, 3 bits type, 6 bits long identifier
        case 2:
            return (1 << 9) - 1;    // 12 unused bits, 3 bits type, 9 bits long identifier
        default:    // 3..7
            return (1 << 21) - 1;   // 0 unused bits, 3 bits type, 21 bits long identifier
    }
}

int NETID::getRFUBitsCount() const
{
    switch (((NETID_TYPE*) this)->networkType) {
        case 0:
        case 1:
            return 15;  // 15 unused bits, 3 bits type, 6 bits identifier
        case 2:
            return 12;  // 12 unused bits, 3 bits type, 9 bits identifier
        default:        // 3..7
            return 0;   // 0 unused bits, 3 bits type, 21 bits identifier
    }
}

int NETID::getNetIdBitsCount() const
{
    switch (((NETID_TYPE*) this)->networkType) {
        case 0:
        case 1:
            return 6;    // 15 unused bits, 3 bits type, 6 bits identifier
        case 2:
            return 9;    // 12 unused bits, 3 bits type, 9 bits identifier
        default:    // 3..7
            return 21;   // 0 unused bits, 3 bits type, 21 bits identifier
    }
}

typedef struct {
    uint8_t networkIdBits;
    uint8_t devDddrBits;
} DEVADDR_TYPE_SIZE;

// version 1.0
static const DEVADDR_TYPE_SIZE DEVADDR_TYPE_SIZES_1_0[8] = {
    { 6, 25 },    // 0
    { 6, 24 },    // 1
    { 9, 20 },    // 2
    { 10, 18 },   // 3
    { 11, 16 },   // 4
    { 13, 13 },   // 5
    { 15, 10 },   // 6
    { 17, 7 }     // 7
};

// version 1.1
static const DEVADDR_TYPE_SIZE DEVADDR_TYPE_SIZES_1_1[8] = {
    { 6, 25 },    // 0
    { 6, 24 },    // 1
    { 9, 20 },    // 2
    { 11, 17 },   // 3
    { 12, 15 },   // 4
    { 13, 13 },   // 5
    { 15, 10 },   // 6
    { 17, 7 }     // 7
};

size_t NETID::size()
{
    uint8_t typ = ((NETID_TYPE*) this)->networkType;
#if DEFAULT_LORAWAN_BACKEND_VERSION_MINOR == 1
    return (1 << DEVADDR_TYPE_SIZES_1_1[typ].devDddrBits) - 1;
#else
    return (1 << DEVADDR_TYPE_SIZES_1_0[typ].devDddrBits) - 1;
#endif
}
// ------------------- NETID ------------------->

// <------------------- DEVADDR -------------------

#define DEFAULT_LORAWAN_BACKEND_VERSION_MINOR   1

DEVADDR::DEVADDR()
    : u(0)
{

}

DEVADDR::DEVADDR(
	const DEVADDR &value
) {
    u = value.u;
}

DEVADDR::DEVADDR(
    uint32_t value
) {
    u = value;
}

DEVADDR::DEVADDR(
    const std::string &value
) {
    string2DEVADDR(*this, value);
}

DEVADDR::DEVADDR(
	const NETID &netid,
	uint32_t nwkAddr
)
{
    set(netid, nwkAddr);
}

DEVADDR::DEVADDR(
	uint8_t netTypeId,
	uint32_t nwkId,
	uint32_t nwkAddr
)
{
    set(netTypeId, nwkId, nwkAddr);
}

/**
 * Maximum nerwork id & address
 * @param netTypeId type identifier
 */
DEVADDR::DEVADDR(
    const NETID &netId,
    bool retMax
) {
    if (retMax)
        setMaxAddress(netId);
    else
        setMinAddress(netId);
}

void DEVADDR::get(
	DEVADDR &retval
) const
{
    retval.u = u;
}

int DEVADDR::setMaxAddress(
	const NETID &netId
)
{
    uint8_t t = netId.getType();
    int r = setNetIdType(t);
    if (r)
        return r;
    r = setNwkId(t, netId.getNwkId());  // getMaxNwkId(t)
    if (r)
        return r;
    return setNwkAddr(netId.getType(), getMaxNwkAddr(t));
}

int DEVADDR::setMinAddress(
	const NETID &netId
)
{
    uint8_t t = netId.getType();
    int r = setNetIdType(netId.getType());
    if (r)
        return r;
    r = setNwkId(t, netId.getNwkId());  // 0
    if (r)
        return r;
    return setNwkAddr(t, 0);
}

size_t DEVADDR::size()
{
    uint8_t typ = getNetIdType();
    if (typ >= 8)
        return 0;
#if DEFAULT_LORAWAN_BACKEND_VERSION_MINOR == 1
    return (1 << DEVADDR_TYPE_SIZES_1_1[typ].devDddrBits) - 1;
#else
    return (1 << DEVADDR_TYPE_SIZES_1_0[typ].devDddrBits) - 1;
#endif
}

std::string DEVADDR::toString() const
{
    return DEVADDR2string(*this);
}

uint32_t DEVADDR::get() const
{
    return *(uint32_t *) this;
}

void DEVADDR::set(
    const std::string &value
)
{
    string2DEVADDR(*this, value);
}

void DEVADDR::set(
	const DEVADDR &value
) {
    u = value.u;
}

void DEVADDR::set(
	uint32_t value
) {
    int2DEVADDR(*this, value);
}

/**
 * DEVADDR has 8 types;
 * 0..2- NwkId = NetId
 * 3..7- NwkId is a part of NetId
 * @see LoRaWAN Backend Interfaces 1.0 Specification
 *      Chapter 13 DevAddr Assignment Table 3
 * @link link-object https://lora-alliance.org/resource_hub/lorawan-back-end-interfaces-v1-0/ @endlink
 * @link link-object https://lora-alliance.org/wp-content/uploads/2020/11/lorawantm-backend-interfaces-v1.0.pdf @endlink
 * @see NetId
 * @return 0..7 NetId type, 8- invalid dev address
 */
uint8_t DEVADDR::getNetIdType() const
{
    uint8_t typePrefix8 = c[3];
    if (typePrefix8 == 0xfe)
        return 7;
    typePrefix8 = typePrefix8 >> 1;
    if (typePrefix8 == 0x7e)
        return 6;
    typePrefix8 = typePrefix8 >> 1;
    if (typePrefix8 == 0x3e)
        return 5;
    typePrefix8 = typePrefix8 >> 1;
    if (typePrefix8 == 0x1e)
        return 4;
    typePrefix8 = typePrefix8 >> 1;
    if (typePrefix8 == 0xe)
        return 3;
    typePrefix8 = typePrefix8 >> 1;
    if (typePrefix8 == 6)
        return 2;
    typePrefix8 = typePrefix8 >> 1;
    if (typePrefix8 == 2)
        return 1;
    typePrefix8 = typePrefix8 >> 1;
    if (typePrefix8 == 0)
        return 0;
    return 8;
}

int DEVADDR::setNetIdType(
	uint8_t value
)
{
    switch (value) {
    case 0:
        c[3] &= 0x7f; // 0b
        break;
    case 1:
        c[3] = (c[3] & 0x3f) | 0x80;   // 10b
        break;
    case 2:
        c[3] = (c[3] & 0x1f) | 0xc0;   // 110b
        break;
    case 3:
        c[3] = (c[3] & 0xf) | 0xe0;   // 1110b
        break;
    case 4:
        c[3] = (c[3] & 7) | 0xf0;  // 11110b
        break;
    case 5:
        c[3] = (c[3] & 3) | 0xf8;  // 111110b
        break;
    case 6:
        c[3] = (c[3] & 1) | 0xfc;  // 1111110b
        break;
    case 7:
        c[3] = 0xfe;  // 11111110b
        break;
    default:
        return ERR_CODE_TYPE_OUT_OF_RANGE;
    }
    return 0;
}

/**
 * @return NwkId
 */
uint32_t DEVADDR::getNwkId() const
{
#if DEFAULT_LORAWAN_BACKEND_VERSION_MINOR == 1
    return getNwkId_1_1();
#else
    return getNwkId_1_0();
#endif
}

/**
 * @return NwkId
 * Type Bits                             Hex
 *      MSB
 * 0    T6666660 00000000 00000000 00000000
 * 1    TT666666 00000000 00000000 00000000
 * 2    TTT99999 99990000 00000000 00000000
 * 3    TTTTAAAA AAAAAA00 00000000 00000000
 * 4    TTTTTBBB BBBBBBBB 00000000 00000000
 * 5    TTTTTTDD DDDDDDDD DDD00000 00000000
 * 6    TTTTTTTF FFFFFFFF FFFFFF00 00000000
 * 7    TTTTTTTT 11111111 11111111 10000000
 */
uint32_t DEVADDR::getNwkId_1_0() const
{
    switch (getNetIdType())
    {
        case 0:
            return (c[3] & 0x7f) >> 1;
        case 1:
            return c[3] & 0x3f;
        case 2:
            return ((c[3] & 0x1f) << 4) | (c[2] >> 4);
        case 3:
            return ((c[3] & 0xf) << 6) | (c[2] >> 2);
        case 4:
            return ((c[3] & 7) << 8) | c[2];
        case 5:
            return ((c[3] & 3) << 11) | (c[2] << 3) | (c[1] >> 5); // (c[1] & 0xe0
        case 6:
            return ((c[3] & 1) << 14) | (c[2] << 6) | (c[1] >> 2); // (c[1] & 0xfc
        case 7:
            return (c[2] << 9) | (c[1] << 1) | (c[0] >> 7);  // (c[0] & 0x80)
        default:
            return INVALID_ID;
    }
}

/**
 * @return NwkId
 * Type Bits                             Hex
 *      MSB
 * 0    T6666660 00000000 00000000 00000000
 * 1    TT666666 00000000 00000000 00000000
 * 2    TTT99999 99990000 00000000 00000000
 * 3    TTTTBBBB BBBBBBB0 00000000 00000000
 * 4    TTTTTCCC CCCCCCCC C0000000 00000000
 * 5    TTTTTTDD DDDDDDDD DDD00000 00000000
 * 6    TTTTTTTF FFFFFFFF FFFFFF00 00000000
 * 7    TTTTTTTT 11111111 11111111 10000000
 */
uint32_t DEVADDR::getNwkId_1_1() const
{
    switch (getNetIdType())
    {
        case 0:
            return (c[3] & 0x7f) >> 1;
        case 1:
            return c[3] & 0x3f;
        case 2:
            return ((c[3] & 0x1f) << 4) | (c[2] >> 4);
        case 3:
            return ((c[3] & 0xf) << 7) | (c[2] >> 1);
        case 4:
            return ((c[3] & 7) << 9) | (c[2] << 1) | (c[0] >> 7);
        case 5:
            return ((c[3] & 3) << 11) | (c[2] << 3) | (c[1] >> 5);
        case 6:
            return ((c[3] & 1) << 14) | (c[2] << 6) | (c[1] >> 2);
        case 7:
            return (c[2] << 9) | (c[1] << 1) | (c[0] >> 7);
        default:
            return INVALID_ID;
    }
}

uint32_t DEVADDR::getNwkAddr() const
{
#if DEFAULT_LORAWAN_BACKEND_VERSION_MINOR == 1
    return getNwkAddr_1_1();
#else
    return getNwkAddr_1_0();
#endif
}

/**
 * LoraWAN backend interfaces Version 1.0
 * @return NwkAddr
 * Type Bits                             Hex
 *      MSB
 * 0    T6666660 00000000 00000000 00000000
 * 1    TT666666 00000000 00000000 00000000
 * 2    TTT99999 99990000 00000000 00000000
 * 3    TTTTAAAA AAAAAA00 00000000 00000000
 * 4    TTTTTBBB BBBBBBBB 00000000 00000000
 * 5    TTTTTTDD DDDDDDDD DDD00000 00000000
 * 6    TTTTTTTF FFFFFFFF FFFFFF00 00000000
 * 7    TTTTTTTT 11111111 11111111 10000000
 */
uint32_t DEVADDR::getNwkAddr_1_0() const
{
    switch (getNetIdType())
    {
    case 0:
        return ((c[3] & 1) << 24) | (c[2] << 16) | (c[1] << 8) | c[0];
    case 1:
        return (c[2] << 16) | (c[1] << 8) | c[0];
    case 2:
        return ((c[2] & 0xf) << 16) | (c[1] << 8) | c[0];
    case 3:
        return ((c[2] & 3) << 16) | (c[1] << 8) | c[0];
    case 4:
        return (c[1] << 8) | c[0];
    case 5:
        return ((c[1] & 0x1f ) << 8) | c[0];
    case 6:
        return ((c[1] & 3) << 8) | c[0];
    case 7:
        return c[0] & 0x7f;
    default:
        return INVALID_ID;
    }
}

/**
 * LoraWAN backend interfaces Version 1.0
 * @return NwkAddr
 * Type Bits                             Hex
 *      MSB
 * 0    T6666660 00000000 00000000 00000000
 * 1    TT666666 00000000 00000000 00000000
 * 2    TTT99999 99990000 00000000 00000000
 * 3    TTTTBBBB BBBBBBB0 00000000 00000000
 * 4    TTTTTCCC CCCCCCCC C0000000 00000000
 * 5    TTTTTTDD DDDDDDDD DDD00000 00000000
 * 6    TTTTTTTF FFFFFFFF FFFFFF00 00000000
 * 7    TTTTTTTT 11111111 11111111 10000000
 */
uint32_t DEVADDR::getNwkAddr_1_1() const
{
    switch (getNetIdType())
    {
        case 0:
            return ((c[3] & 1) << 24) | (c[2] << 16) | (c[1] << 8) | c[0];
        case 1:
            return (c[2] << 16) | (c[1] << 8) | c[0];
        case 2:
            return ((c[2] & 0xf) << 16) | (c[1] << 8) | c[0];
        case 3:
            return ((c[2] & 1) << 16) | (c[1] << 8) | c[0];
        case 4:
            return ((c[1] & 0x7f)<< 8) | c[0];
        case 5:
            return ((c[1] & 0x1f ) << 8) | c[0];
        case 6:
            return ((c[1] & 3) << 8) | c[0];
        case 7:
            return c[0] & 0x7f;
        default:
            return INVALID_ID;
    }
}

/*
 * Version 1.0
 * NwkId clear mask
 * Type Bits                             Hex
 *      MSB
 * 0    10000001111111111111111111111111 0x81FFFFFF
 * 1    11000000111111111111111111111111 0xC0FFFFFF
 * 2    11100000000011111111111111111111 0xE00FFFFF
 * 3    11110000000000111111111111111111 0xF003FFFF
 * 4    11111000000000001111111111111111 0xF800FFFF
 * 5    11111100000000000111111111111111 0xFC007FFF
 * 6    11111110000000000000001111111111 0xFE0003FF
 * 7    11111111000000000000000001111111 0xFF00007F
 */

#define NWKID_CLEAR_1_0_0 0x81FFFFFF
#define NWKID_CLEAR_1_0_1 0xC0FFFFFF
#define NWKID_CLEAR_1_0_2 0xE00FFFFF
#define NWKID_CLEAR_1_0_3 0xF003FFFF
#define NWKID_CLEAR_1_0_4 0xF800FFFF
#define NWKID_CLEAR_1_0_5 0xFC007FFF
#define NWKID_CLEAR_1_0_6 0xFE0003FF
#define NWKID_CLEAR_1_0_7 0xFF00007F

/*
 * Version 1.1
 * NwkId clear mask
 * Type Bits                             Hex
 *      MSB
 * 0    10000001111111111111111111111111 0x81FFFFFF
 * 1    11000000111111111111111111111111 0xC0FFFFFF
 * 2    11100000000011111111111111111111 0xE00FFFFF
 * 3    11110000000000011111111111111111 0xF001FFFF
 * 4    11111000000000000111111111111111 0xF8007FFF
 * 5    11111100000000000111111111111111 0xFC007FFF
 * 6    11111110000000000000001111111111 0xFE0003FF
 * 7    11111111000000000000000001111111 0xFF00007F
 */

#define NWKID_CLEAR_1_1_0 0x81FFFFFF
#define NWKID_CLEAR_1_1_1 0xC0FFFFFF
#define NWKID_CLEAR_1_1_2 0xE00FFFFF
#define NWKID_CLEAR_1_1_3 0xF001FFFF
#define NWKID_CLEAR_1_1_4 0xF8007FFF
#define NWKID_CLEAR_1_1_5 0xFC007FFF
#define NWKID_CLEAR_1_1_6 0xFE0003FF
#define NWKID_CLEAR_1_1_7 0xFF00007F

#define DEVADDR_SET_NWK_ID_1_0(typ, u32, prefix_n_mnwkid_len) \
    u = (u & NWKID_CLEAR_1_0_ ## typ ) | (u32 << (32 - prefix_n_mnwkid_len))

#define DEVADDR_SET_NWK_ID_1_1(typ, u32, prefix_n_mnwkid_len) \
    u = (u & NWKID_CLEAR_1_1_ ## typ ) | (u32 << (32 - prefix_n_mnwkid_len))

/*
 * LoraWAN beckend interfaces Version 1.0
 * NwkAddr clear mask
 * Type Bits                             Hex
 *      MSB
 * 0    11111110000000000000000000000000 0xFE000000
 * 1    11111111000000000000000000000000 0xFF000000
 * 2    11111111111100000000000000000000 0xFFF00000
 * 3    11111111111111000000000000000000 0xFFFC0000
 * 4    11111111111111110000000000000000 0xFFFF0000
 * 5    11111111111111111000000000000000 0xFFFF8000
 * 6    11111111111111111111110000000000 0xFFFFFC00
 * 7    11111111111111111111111110000000 0xFFFFFF80
 */
#define NWKADDR_CLEAR_V1_0_0 0xFE000000
#define NWKADDR_CLEAR_V1_0_1 0xFF000000
#define NWKADDR_CLEAR_V1_0_2 0xFFF00000
#define NWKADDR_CLEAR_V1_0_3 0xFFFC0000
#define NWKADDR_CLEAR_V1_0_4 0xFFFF0000
#define NWKADDR_CLEAR_V1_0_5 0xFFFF8000
#define NWKADDR_CLEAR_V1_0_6 0xFFFFFC00
#define NWKADDR_CLEAR_V1_0_7 0xFFFFFF80

#define DEVADDR_SET_NWK_ADDR_1_0(typ, u32) \
    u = (u & NWKADDR_CLEAR_V1_0_ ## typ ) | u32

/*
 * LoraWAN beckend interfaces Version 1.1
 * NwkAddr clear mask
 * Type Bits                             Hex
 *      MSB
 * 0    11111110000000000000000000000000 0xFE000000
 * 1    11111111000000000000000000000000 0xFF000000
 * 2    11111111111100000000000000000000 0xFFF00000
 * 3    11111111111111100000000000000000 0xFFFE0000
 * 4    11111111111111111000000000000000 0xFFFF8000
 * 5    11111111111111111000000000000000 0xFFFF8000
 * 6    11111111111111111111110000000000 0xFFFFFC00
 * 7    11111111111111111111111110000000 0xFFFFFF80
 */
#define NWKADDR_CLEAR_V1_1_0 0xFE000000
#define NWKADDR_CLEAR_V1_1_1 0xFF000000
#define NWKADDR_CLEAR_V1_1_2 0xFFF00000
#define NWKADDR_CLEAR_V1_1_3 0xFFFE0000
#define NWKADDR_CLEAR_V1_1_4 0xFFFF8000
#define NWKADDR_CLEAR_V1_1_5 0xFFFF8000
#define NWKADDR_CLEAR_V1_1_6 0xFFFFFC00
#define NWKADDR_CLEAR_V1_1_7 0xFFFFFF80

#define DEVADDR_SET_NWK_ADDR_1_1(typ, u32) \
    u = (u & NWKADDR_CLEAR_V1_1_ ## typ ) | u32

/*
 * LoraWAN beckend interfaces Version 1.1
 * NwkAddr clear mask
 * Type Bits                             Hex
 *      MSB
 * 0    11111110000000000000000000000000 0xFE000000
 * 1    11111111000000000000000000000000 0xFF000000
 * 2    11111111111100000000000000000000 0xFFF00000
 * 3    11111111111111100000000000000000 0xFFFE0000
 * 4    11111111111111111000000000000000 0xFFFF8000
 * 5    11111111111111111000000000000000 0xFFFF8000
 * 6    11111111111111111111110000000000 0xFFFFFC00
 * 7    11111111111111111111111110000000 0xFFFFFF80
 */
#define NWKADDR_CLEAR_V1_1_0 0xFE000000
#define NWKADDR_CLEAR_V1_1_1 0xFF000000
#define NWKADDR_CLEAR_V1_1_2 0xFFF00000
#define NWKADDR_CLEAR_V1_1_3 0xFFFE0000
#define NWKADDR_CLEAR_V1_1_4 0xFFFF8000
#define NWKADDR_CLEAR_V1_1_5 0xFFFF8000
#define NWKADDR_CLEAR_V1_1_6 0xFFFFFC00
#define NWKADDR_CLEAR_V1_1_7 0xFFFFFF80

int DEVADDR::setNwkId(
	uint8_t netIdType,
	uint32_t value
)
{
#if DEFAULT_LORAWAN_BACKEND_VERSION_MINOR == 1
    return setNwkId_1_1(netIdType, value);
#else
    return setNwkId_1_0(netIdType, value);
#endif
}

int DEVADDR::setNwkId_1_0(
	uint8_t netIdType,
	uint32_t value
)
{
    switch (netIdType) {
        case 0:
            if (value >= 0x40)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_0(0, value, 7);
            break;
        case 1:
            if (value >= 0x40)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_0(1, value, 8);
            break;
        case 2:
            if (value >= 0x200)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_0(2, value, 12);
            break;
        case 3:
            if (value >= 0x400)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_0(3, value, 14);
            break;
        case 4:
            if (value >= 0x800)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_0(4, value, 16);
            break;
        case 5:
            if (value >= 0x2000)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_0(5, value, 19);
            break;
        case 6:
            if (value >= 0x8000)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_0(6, value, 22);
            break;
        case 7:
            if (value >= 0x20000)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_0(7, value, 25);
            break;
        default:
            return ERR_CODE_NWK_OUT_OF_RANGE;
    }
    return 0;
}

int DEVADDR::setNwkId_1_1(
	uint8_t netIdType,
	uint32_t value
)
{
    switch (netIdType) {
        case 0:
            if (value >= 0x40)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_1(0, value, 7);
            break;
        case 1:
            if (value >= 0x40)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_1(1, value, 8);
            break;
        case 2:
            if (value >= 0x200)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_1(2, value, 12);
            break;
        case 3:
            if (value >= 0x800)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_1(3, value, 14);
            break;
        case 4:
            if (value >= 0x1000)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_1(4, value, 16);
            break;
        case 5:
            if (value >= 0x2000)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_1(5, value, 19);
            break;
        case 6:
            if (value >= 0x8000)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_1(6, value, 22);
            break;
        case 7:
            if (value >= 0x20000)
                return ERR_CODE_NWK_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ID_1_1(7, value, 25);
            break;
        default:
            return ERR_CODE_NWK_OUT_OF_RANGE;
    }
    return 0;
}

int DEVADDR::setNwkAddr(
	uint8_t netIdType,
	uint32_t value
)
{
#if DEFAULT_LORAWAN_BACKEND_VERSION_MINOR == 1
    return setNwkAddr_1_1(netIdType, value);
#else
    return setNwkAddr_1_0(netIdType, value);
#endif
}

int DEVADDR::setNwkAddr_1_0(
	uint8_t netIdType,
	uint32_t value
)
{
    switch (netIdType) {
        case 0:
            if (value >= 0x2000000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_0(0, value);
            break;
        case 1:
            if (value >= 0x1000000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_0(1, value);
            break;
        case 2:
            if (value >= 0x100000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_0(2, value);
            break;
        case 3:
            if (value >= 0x40000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_0(3, value);
            break;
        case 4:
            if (value >= 0x10000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_0(4, value);
            break;
        case 5:
            if (value >= 0x2000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_0(5, value);
            break;
        case 6:
            if (value >= 0x400)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_0(6, value);
            break;
        case 7:
            if (value >= 0x80)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_0(7, value);
            break;
        default:
            return ERR_CODE_ADDR_OUT_OF_RANGE;
    }
    return 0;
}
int DEVADDR::setNwkAddr_1_1(
	uint8_t netIdType,
	uint32_t value
)
{
    switch (netIdType) {
        case 0:
            if (value >= 0x2000000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_1(0, value);
            break;
        case 1:
            if (value >= 0x1000000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_1(1, value);
            break;
        case 2:
            if (value >= 0x100000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_1(2, value);
            break;
        case 3:
            if (value >= 0x20000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_1(3, value);
            break;
        case 4:
            if (value >= 0x8000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_1(4, value);
            break;
        case 5:
            if (value >= 0x2000)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_1(5, value);
            break;
        case 6:
            if (value >= 0x400)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_1(6, value);
            break;
        case 7:
            if (value >= 0x80)
                return ERR_CODE_ADDR_OUT_OF_RANGE;
            DEVADDR_SET_NWK_ADDR_1_1(7, value);
            break;
        default:
            return ERR_CODE_ADDR_OUT_OF_RANGE;
    }
    return 0;
}

int DEVADDR::set(
    uint8_t netTypeId, 
    uint32_t nwkId,
    uint32_t nwkAddr
)
{
    int r = setNetIdType(netTypeId);
    if (r)
        return r;
    r = setNwkId(netTypeId, nwkId);
    if (r)
        return r;
    return setNwkAddr(netTypeId, nwkAddr);
}

// Set address only (w/o nwkId)
int DEVADDR::setAddr(
    uint32_t nwkAddr
) {
    return setNwkAddr(getNetIdType(), nwkAddr);
}

int DEVADDR::set(
    const NETID &netid,
    uint32_t nwkAddr
)
{
    return set(netid.getType(), netid.getNwkId(), nwkAddr);
}

std::size_t DEVADDR::operator()(
	const DEVADDR &value
) const {
	return value.u;
}

bool DEVADDR::operator==(
	const DEVADDR &rhs
) const {
	return rhs.u == u;
}

bool DEVADDR::operator<(
	const DEVADDR &rhs
) const {
	return u < rhs.u;
}

bool DEVADDR::operator>(
	const DEVADDR &rhs
) const {
	return u > rhs.u;
}

bool DEVADDR::operator<=(
    const DEVADDR &rhs
) const {
    return u <= rhs.u;
}

bool DEVADDR::operator>=(
    const DEVADDR &rhs
) const {
    return u >= rhs.u;
}

bool DEVADDR::operator!=(
	const DEVADDR &rhs
) const {
	return rhs.u != u;
}

bool DEVADDR::empty() const
{
    return isDEVADDREmpty(*this);
}

// prefix increment operator
DEVADDR& DEVADDR::operator++()
{
    increment();
    return *this;
}

// prefix decrement operator
DEVADDR& DEVADDR::operator--()
{
    decrement();
    return *this;
}

DEVADDR& DEVADDR::operator=(
    const DEVADDR& value
)
{
    u = value.u;
    return *this;
}

int DEVADDR::increment()
{
    return setAddr(getNwkAddr() + 1);
}

int DEVADDR::decrement()
{
    return setAddr(getNwkAddr() - 1);
}

uint32_t DEVADDR::getMaxNwkId(uint8_t netTypeId) {
    switch (netTypeId)
    {
        case 0:
            return 0x40 - 1;
        case 1:
            return 0x40 - 1;
        case 2:
            return 0x200 - 1;
        case 3:
            return 0x400 - 1;
        case 4:
            return 0x800 - 1;
        case 5:
            return 0x2000 - 1;
        case 6:
            return 0x8000 - 1;
        case 7:
            return 0x20000 - 1;
        default:
            return ERR_CODE_NWK_OUT_OF_RANGE;
    }
}

uint32_t DEVADDR::getMaxNwkAddr(uint8_t netTypeId) {
#if DEFAULT_LORAWAN_BACKEND_VERSION_MINOR == 1
    return getMaxNwkAddr_1_1(netTypeId);
#else
    return getMaxNwkAddr_1_0(netTypeId);
#endif
}

uint32_t DEVADDR::getMaxNwkAddr_1_0(uint8_t netTypeId) {
    switch (netTypeId)
    {
        case 0:
            return 0x2000000 - 1;
        case 1:
            return 0x1000000 - 1;
        case 2:
            return 0x100000 - 1;
        case 3:
            return 0x40000 - 1;
        case 4:
            return 0x10000 - 1;
        case 5:
            return 0x2000 - 1;
        case 6:
            return 0x400 - 1;
        case 7:
            return 0x80 - 1;
        default:
            return ERR_CODE_ADDR_OUT_OF_RANGE;
    }
}

uint32_t DEVADDR::getMaxNwkAddr_1_1(uint8_t netTypeId) {
    switch (netTypeId)
    {
        case 0:
            return 0x2000000 - 1;
        case 1:
            return 0x1000000 - 1;
        case 2:
            return 0x100000 - 1;
        case 3:
            return 0x20000 - 1;
        case 4:
            return 0x8000 - 1;
        case 5:
            return 0x2000 - 1;
        case 6:
            return 0x400 - 1;
        case 7:
            return 0x80 - 1;
        default:
            return ERR_CODE_ADDR_OUT_OF_RANGE;
    }
}

uint8_t DEVADDR::getTypePrefixBitsCount(
    uint8_t netTypeId
)
{
    return netTypeId + 1;
}

uint8_t DEVADDR::getNwkIdBitsCount(
	uint8_t typ
) {
#if DEFAULT_LORAWAN_BACKEND_VERSION_MINOR == 1
    return DEVADDR_TYPE_SIZES_1_1[typ].networkIdBits;
#else
    return DEVADDR_TYPE_SIZES_1_0[typ].networkIdBits;
#endif
}

uint8_t DEVADDR::getNwkAddrBitsCount(
	uint8_t typ
) {
#if DEFAULT_LORAWAN_BACKEND_VERSION_MINOR == 1
    return DEVADDR_TYPE_SIZES_1_1[typ].devDddrBits;
#else
    return DEVADDR_TYPE_SIZES_1_0[typ].devDddrBits;
#endif
}

// ------------------- DEVADDR ------------------->

KEY128::KEY128()
{
    memset(&c, 0, 16);
}
KEY128::KEY128(
    const std::string &hex
) {
    string2KEY(*this, hex);
}

KEY128::KEY128(
    const char* hex
)
{
    string2KEY(*this, hex);
}

KEY128::KEY128(
    uint64_t hi,
    uint64_t lo
)
{
#if BYTE_ORDER == BIG_ENDIAN
    u[0] = lo;
    u[1] = hi;
#else
    u[1] = NTOH8(lo);
    u[0] = NTOH8(hi);
#endif

}

KEY128::KEY128(
    const KEY128 &value
)
{
    memmove(&c, &value, 16);
}

std::size_t KEY128::operator()(const KEY128 &value) const {
    return std::hash<std::size_t>{}(u[0] ^ u[1]);
}

bool KEY128::operator==(const KEY128 &rhs) const {
    return memcmp(&c, &rhs.c, 16) == 0;
}
bool KEY128::operator<(const KEY128 &rhs) const {
    return memcmp(&c, &rhs.c, 16) < 0;
}
bool KEY128::operator>(const KEY128 &rhs) const {
    return memcmp(&c, &rhs.c, 16) > 0;
}
bool KEY128::operator!=(const KEY128 &rhs) const {
    return memcmp(&rhs.c, &c, 16) != 0;
}

DEVEUI::DEVEUI()
    : u(0)
{

}
DEVEUI::DEVEUI(
    const std::string &hex
) {
    string2DEVEUI(*this, hex);
}

DEVEUI::DEVEUI(
    uint64_t value
)
{
    u = value;
}

bool DEVEUI::operator==(const DEVEUI &rhs) const {
    return rhs.u == u;
}
bool DEVEUI::operator<(const DEVEUI &rhs) const {
    return u < rhs.u;
}
bool DEVEUI::operator>(const DEVEUI &rhs) const {
    return u > rhs.u;
}
bool DEVEUI::operator!=(const DEVEUI &rhs) const {
    return rhs.u != u;
}

JOINNONCE::JOINNONCE()
{
    c[0] = 0;
    c[1] = 0;
    c[2] = 0;
}

JOINNONCE::JOINNONCE(
    const std::string &hex
)
{
    string2JOINNONCE(*this, hex);
}

JOINNONCE::JOINNONCE(
    uint32_t value
)
{
    int2JOINNONCE(*this, value);
}

uint32_t JOINNONCE::get() const
{
    return JOINNONCE2int(*this);
}

APPNONCE::APPNONCE()
{
    c[0] = 0;
    c[1] = 0;
    c[2] = 0;
}

APPNONCE::APPNONCE(
    const std::string& hex
)
{
    string2APPNONCE(*this, hex);
}

APPNONCE::APPNONCE(
    uint32_t value
)
{
    int2APPNONCE(*this, value);
}

DEVNONCE::DEVNONCE()
    : u(0)
{
}

DEVNONCE::DEVNONCE(
    const std::string& hex
)
{
    *this = string2DEVNONCE(hex);
}

DEVNONCE::DEVNONCE(
    uint16_t value
)
{
    u = value;
}

DEVICENAME::DEVICENAME()
{
    memset(&c, 0, 8);
}

DEVICENAME::DEVICENAME(
    const std::string &value
)
{
    size_t sz = value.length();
    if (sz > 8)
        sz = 8;
    strncpy(c, value.c_str(), sz);
}

DEVICENAME::DEVICENAME(
    const char *value
)
{
    size_t sz = strnlen(value, 8);
    strncpy(c, value, sz);
    // set \0 terminator
    if (sz < 8)
        memset(c + sz, 0, 8 - sz);
}

std::string DEVICENAME::toString() const
{
    return DEVICENAME2string(*this);
}

bool DEVICENAME::empty() const
{
    return c[0] == 0;
}

DEVICEID::DEVICEID() {
    this->id.activation = ABP;
    this->id.deviceclass = CLASS_A;
    memset(&this->id.devEUI.c, 0, sizeof(DEVEUI));
    memset(&this->id.nwkSKey.c, 0, sizeof(KEY128));
    memset(&this->id.appSKey.c, 0, sizeof(KEY128));
    
    memset(&this->id.appEUI.c, 0, sizeof(DEVEUI));
    memset(&this->id.nwkKey.c, 0, sizeof(KEY128));
    memset(&this->id.appKey.c, 0, sizeof(KEY128));
    id.devNonce.u = 0;
    memset(&this->id.joinNonce.c, 0, sizeof(JOINNONCE));
    id.token = 0;
    id.region = 0;
    id.subRegion = 0;
    memset(&this->id.name.c, 0, sizeof(DEVICENAME));
    id.version.major = 1;
}

DEVICEID::DEVICEID(
    const DEVEUI &deveui
)
{
    id.devEUI = deveui;
    id.activation = ABP;
    id.deviceclass = CLASS_A;

    memset(&this->id.nwkSKey.c, 0, sizeof(KEY128));
    memset(&this->id.appSKey.c, 0, sizeof(KEY128));

    memset(&this->id.appEUI.c, 0, sizeof(DEVEUI));
    memset(&this->id.nwkKey.c, 0, sizeof(KEY128));
    memset(&this->id.appKey.c, 0, sizeof(KEY128));
    id.devNonce.u = 0;
    memset(&this->id.joinNonce.c, 0, sizeof(JOINNONCE));
    id.token = 0;
    id.region = 0;
    id.subRegion = 0;
    memset(&this->id.name.c, 0, sizeof(DEVICENAME));
    id.version.major = 1;
}

DEVICEID::DEVICEID(
    ACTIVATION activation,
    DEVICECLASS deviceclass,
    const DEVEUI &devEUI,
    const KEY128 &nwkSKey,
    const KEY128 &appSKey,
    LORAWAN_VERSION version,
    const DEVEUI &appEUI,
    const KEY128 &appKey,
    const KEY128 &nwkKey,
    DEVNONCE devNonce,
    const JOINNONCE joinNonce,
    uint32_t token,
    uint8_t region,
    uint8_t subRegion,
    const DEVICENAME name
) {
    this->id.activation = activation;
    this->id.deviceclass = deviceclass;
    memmove(&this->id.devEUI.c, &devEUI.c, sizeof(DEVEUI));
    memmove(&this->id.nwkSKey.c, &nwkSKey.c, sizeof(KEY128));
    memmove(&this->id.appSKey.c, &appSKey.c, sizeof(KEY128));
    this->id.version = version;
    memmove(&this->id.appEUI.c, &appEUI.c, sizeof(DEVEUI));
    memmove(&this->id.nwkKey.c, &nwkKey.c, sizeof(KEY128));
    memmove(&this->id.appKey.c, &appKey.c, sizeof(KEY128));
    this->id.devNonce = devNonce;
    memmove(&this->id.joinNonce.c, &joinNonce.c, sizeof(JOINNONCE));
    id.token = token;
    id.region = region;
    id.subRegion = subRegion;
    memmove(&this->id.name, &name, sizeof(DEVICENAME));
}

DEVICEID::DEVICEID(
    DEVICECLASS deviceclass,
    const DEVEUI &devEUI,
    const KEY128 &nwkSKey,
    const KEY128 &appSKey,
    LORAWAN_VERSION version,
    uint32_t token,
    uint8_t region,
    uint8_t subRegion,
    const DEVICENAME name
) {
    id.activation = ABP;
    id.deviceclass = deviceclass;
    memmove(&this->id.devEUI.c, &devEUI.c, sizeof(DEVEUI));
    memmove(&this->id.nwkSKey.c, &nwkSKey.c, sizeof(KEY128));
    memmove(&this->id.appSKey.c, &appSKey.c, sizeof(KEY128));
    id.version = version;
    memset(&this->id.appEUI.c, 0, sizeof(DEVEUI));
    memset(&this->id.nwkKey.c, 0, sizeof(KEY128));
    memset(&this->id.appKey.c, 0, sizeof(KEY128));
    id.devNonce.u = 0;
    memset(&id.joinNonce.c, 0, sizeof(JOINNONCE));
    id.token = token;
    id.region = region;
    id.subRegion = subRegion;
    memmove(&this->id.name, &name, sizeof(DEVICENAME));
}

DEVICEID::DEVICEID(
    uint64_t did
)
{
    id.devEUI.u = did;
    this->id.activation = ABP;
    this->id.deviceclass = CLASS_A;
    memset(&this->id.devEUI.c, 0, sizeof(DEVEUI));
    memset(&this->id.nwkSKey.c, 0, sizeof(KEY128));
    memset(&this->id.appSKey.c, 0, sizeof(KEY128));

    memset(&this->id.appEUI.c, 0, sizeof(DEVEUI));
    memset(&this->id.nwkKey.c, 0, sizeof(KEY128));
    memset(&this->id.appKey.c, 0, sizeof(KEY128));
    this->id.devNonce.u = 0;
    memset(&this->id.joinNonce.c, 0, sizeof(JOINNONCE));
    memset(&this->id.name.c, 0, sizeof(DEVICENAME));
    id.version.major = 1;
    id.token = 0;
    id.region = 0;
    id.subRegion = 0;
}

DEVICEID::DEVICEID(
	const DEVICEID &value
)
{
	set(value);
}

DEVICEID& DEVICEID::operator=(
    const DEVICEID& value
)
{
	if (this == &value)
		return *this;
    id.activation = value.id.activation;	///< activation type: ABP or OTAA
    id.deviceclass = value.id.deviceclass;
	memmove(&id.devEUI.c, &value.id.devEUI.c, sizeof(DEVEUI));
	memmove(&id.nwkSKey.c, &value.id.nwkSKey.c, sizeof(KEY128));
	memmove(&id.appSKey.c, &value.id.appSKey.c, sizeof(KEY128));
    id.version = value.id.version;		///< device LoraWAN version
    memmove(&id.appEUI.c, &value.id.appEUI.c, sizeof(DEVEUI));
    memmove(&id.appKey.c, &value.id.appKey.c, sizeof(KEY128));
    memmove(&id.nwkKey.c, &value.id.nwkKey.c, sizeof(KEY128));
    id.devNonce = value.id.devNonce;
    memmove(&id.joinNonce.c, &value.id.joinNonce.c, sizeof(JOINNONCE));
	memmove(&id.name, &value.id.name, sizeof(DEVICENAME));
    id.token = value.id.token;
    id.region = value.id.region;
    id.subRegion = value.id.subRegion;

	return *this;
}

DEVICEID& DEVICEID::operator=(const NETWORKIDENTITY& value)
{
    set(value.value.devid);
	return *this;
}

void DEVICEID::set(
	const DEVICEID &value
)
{
	memmove(&id.activation, &value.id.activation, sizeof(id.activation));
	memmove(&id.deviceclass, &value.id.deviceclass, sizeof(id.deviceclass));
	memmove(&id.devEUI.c, &value.id.devEUI.c, sizeof(DEVEUI));
	memmove(&id.nwkSKey.c, &value.id.nwkSKey.c, sizeof(KEY128));
	memmove(&id.appSKey.c, &value.id.appSKey.c, sizeof(KEY128));
	memmove(&id.version, &value.id.version.c, sizeof(LORAWAN_VERSION));
	memmove(&id.appEUI.c, &value.id.appEUI.c, sizeof(DEVEUI));
	memmove(&id.appKey.c, &value.id.appKey.c, sizeof(KEY128));
	memmove(&id.nwkKey.c, &value.id.nwkKey.c, sizeof(KEY128));
    id.devNonce = value.id.devNonce;
	memmove(&id.joinNonce.c, &value.id.joinNonce.c, sizeof(JOINNONCE));
    id.token = value.id.token;
    id.region = value.id.region;
    id.subRegion = value.id.subRegion;

	memmove(&id.name, &value.id.name, sizeof(DEVICENAME));
}

void DEVICEID::setEUIString
(
	const std::string &value
)
{
	string2DEVEUI(id.devEUI, value);
}

void DEVICEID::setNwkSKeyString
(
	const std::string &value
)
{
	string2KEY(id.nwkSKey, value);
}

void DEVICEID::setAppSKeyString(
	const std::string &value
)
{
	string2KEY(id.appSKey, value);
}

void DEVICEID::setName(
	const std::string &value
)
{
	string2DEVICENAME(id.name, value.c_str());
}

void DEVICEID::setClass(
	const DEVICECLASS &value
)
{
    id.deviceclass = value;
}

std::string DEVICEID::toString() const
{
    DEVADDR a;
	return toString(a);
}

std::string DEVICEID::toString(
    const DEVADDR &addr
) const
{
    std::stringstream ss;
    if (!addr.empty())
        ss << DEVADDR2string(addr) << LIST_SEPARATOR;
    ss
        << activation2string(id.activation)
        << LIST_SEPARATOR << deviceclass2string(id.deviceclass)
        << LIST_SEPARATOR << DEVEUI2string(id.devEUI)
        << LIST_SEPARATOR << KEY2string(id.nwkSKey)
        << LIST_SEPARATOR << KEY2string(id.appSKey)
        << LIST_SEPARATOR << LORAWAN_VERSION2string(id.version)
        << LIST_SEPARATOR << DEVEUI2string(id.appEUI)
        << LIST_SEPARATOR << KEY2string(id.appKey)
        << LIST_SEPARATOR << KEY2string(id.nwkKey)
        << LIST_SEPARATOR << DEVNONCE2string(id.devNonce)
        << LIST_SEPARATOR << JOINNONCE2string(id.joinNonce)
        << LIST_SEPARATOR << token2string(id.token)
        << LIST_SEPARATOR << region2string(id.region)
        << LIST_SEPARATOR << subRegion2string(id.subRegion)
        << LIST_SEPARATOR << DEVICENAME2string(id.name);
    return ss.str();
}

std::string DEVICEID::toJsonString() const
{
    DEVADDR a;
    return toJsonString(a);
}

std::string DEVICEID::toJsonString(
    const DEVADDR &addr
) const
{
    std::stringstream ss;
    ss << "{";
    if (!addr.empty())
        ss << R"("addr":")" << DEVADDR2string(addr) << "\",";
    ss << R"("activation":")" << activation2string(id.activation)
       << R"(","class":")" << deviceclass2string(id.deviceclass)
       << R"(","deveui":")" << DEVEUI2string(id.devEUI)
       << R"(","nwkSKey":")" << KEY2string(id.nwkSKey)
       << R"(","appSKey":")" << KEY2string(id.appSKey)
       << R"(","version":")" << LORAWAN_VERSION2string(id.version)
       << R"(","appeui":")" << DEVEUI2string(id.appEUI)
       << R"(","appKey":")" << KEY2string(id.appKey)
       << R"(","nwkKey":")" << KEY2string(id.nwkKey)
       << R"(","devNonce":")" << DEVNONCE2string(id.devNonce)
       << R"(","joinNonce":")" << JOINNONCE2string(id.joinNonce)
       << R"(","token":)" << token2string(id.token)
       << R"(,"region":)" << region2string(id.region)
       << R"(,"subRegion":)" << subRegion2string(id.subRegion)
       << R"(,"name":")" << id.name.toString()
       << "\"}";
    return ss.str();
}

void DEVICEID::toArray(
    void *buffer,
    size_t size
) const
{
    if (!buffer || size < SIZE_DEVICEID)
        return;
    char *p = (char*) buffer;
    memmove(p, &id.activation, 1);
    p++;
    memmove(p, &id.deviceclass, 1);
    p++;
    memmove(p, &id.devEUI.c, sizeof(DEVEUI));
    p+= sizeof(DEVEUI);
    memmove(p, &id.nwkSKey.c, sizeof(KEY128));
    p+= sizeof(KEY128);
    memmove(p, &id.appSKey.c, sizeof(KEY128));
    p+= sizeof(KEY128);
    memmove(p, &id.version, sizeof(LORAWAN_VERSION));
    p+= sizeof(LORAWAN_VERSION);
    memmove(p, &id.appEUI.c, sizeof(DEVEUI));
    p+= sizeof(DEVEUI);
    memmove(p, &id.appKey.c, sizeof(KEY128));
    p+= sizeof(KEY128);
    memmove(p, &id.nwkKey.c, sizeof(KEY128));
    p+= sizeof(KEY128);
    memmove(p, &id.devNonce, sizeof(DEVNONCE));
    p+= sizeof(DEVNONCE);
    memmove(p, &id.joinNonce.c, sizeof(JOINNONCE));
    p+= sizeof(JOINNONCE);
    memmove(p, &id.token, sizeof(uint32_t));
    p+= sizeof(uint32_t);
    memmove(p, &id.region, sizeof(uint8_t));
    p+= sizeof(uint8_t);
    memmove(p, &id.subRegion, sizeof(uint8_t));
    p+= sizeof(uint8_t);
    memmove(p, &id.name, sizeof(DEVICENAME));
}

void DEVICEID::fromArray(
    const void *buffer,
    size_t size
)
{
    if (!buffer || size < SIZE_DEVICEID)
        return;
    char *p = (char*) buffer;
    memmove(&id.activation, p, 1);
    p++;
    memmove(&id.deviceclass, p, 1);
    p++;
    memmove(&id.devEUI.c, p, sizeof(DEVEUI));
    p+= sizeof(DEVEUI);
    memmove(&id.nwkSKey.c, p, sizeof(KEY128));
    p+= sizeof(KEY128);
    memmove(&id.appSKey.c, p, sizeof(KEY128));
    p+= sizeof(KEY128);
    memmove(&id.version, p, sizeof(LORAWAN_VERSION));
    p+= sizeof(LORAWAN_VERSION);
    memmove(&id.appEUI.c, p, sizeof(DEVEUI));
    p+= sizeof(DEVEUI);
    memmove(&id.appKey.c, p, sizeof(KEY128));
    p+= sizeof(KEY128);
    memmove(&id.nwkKey.c, p, sizeof(KEY128));
    p+= sizeof(KEY128);
    memmove(&id.devNonce, p, sizeof(DEVNONCE));
    p+= sizeof(DEVNONCE);
    memmove(&id.joinNonce.c, p, sizeof(JOINNONCE));
    p+= sizeof(JOINNONCE);
    memmove(&id.token, p, sizeof(uint32_t));
    p+= sizeof(uint32_t);
    memmove(&id.region, p, sizeof(uint8_t));
    p+= sizeof(uint8_t);
    memmove(&id.subRegion, p, sizeof(uint8_t));
    p+= sizeof(uint8_t);
    memmove(&id.name, p, sizeof(DEVICENAME));
}

void DEVICEID::setProperties
(
	std::map<std::string, std::string> &retval
) const
{
	retval["activation"] = activation2string(id.activation);
	retval["class"] = deviceclass2string(id.deviceclass);
	retval["deveui"] = DEVEUI2string(id.devEUI);
	retval["version"] = LORAWAN_VERSION2string(id.version);
    retval["appeui"] = DEVEUI2string(id.appEUI);
    retval["appKey"] = KEY2string(id.appKey);
    retval["nwkKey"] = KEY2string(id.nwkKey);
    retval["devNonce"] = DEVNONCE2string(id.devNonce);
    retval["joinNonce"] = JOINNONCE2string(id.joinNonce);
    retval["token"] = token2string(id.token);
    retval["region"] = token2string(id.region);
    retval["subRegion"] = token2string(id.subRegion);
	retval["name"] = DEVICENAME2string(id.name);
}

bool DEVICEID::empty() const
{
    return id.devEUI.u == 0;
}

NETWORKIDENTITY::NETWORKIDENTITY()
= default;

NETWORKIDENTITY::NETWORKIDENTITY(
    const DEVADDR &a,
    const DEVICEID &id
)
{
    value.devaddr.u = a.u;
    value.devid = id;
}

NETWORKIDENTITY::NETWORKIDENTITY(
    const NETWORKIDENTITY &id
)
    : value { id.value.devaddr, id.value.devid }
{
}

NETWORKIDENTITY::NETWORKIDENTITY(
    const DEVICEID &id
)
{
    set(DEVADDR(0), id);
}

NETWORKIDENTITY::NETWORKIDENTITY(
    const DEVADDR &addr
)
{
    set(addr, 0);
}

void NETWORKIDENTITY::set(
    const NETWORKIDENTITY &id
)
{
    set(id.value.devaddr, id.value.devid);
}

void NETWORKIDENTITY::set(
    const DEVADDR &addr,
    const DEVICEID &val
)
{
    value.devaddr.u = addr.u;
    value.devid.set(val);
}

std::string NETWORKIDENTITY::toString() const 
{
    return value.devid.toString(value.devaddr);
}

std::string NETWORKIDENTITY::toJsonString() const
{
	return value.devid.toJsonString(value.devaddr);
}

bool JOIN_REQUEST_FRAME::operator==(const JOIN_REQUEST_FRAME &rhs) const
{
    return rhs.joinEUI.u == joinEUI.u && rhs.devEUI.u == devEUI.u;
}

bool JOIN_REQUEST_FRAME::operator<(const JOIN_REQUEST_FRAME &rhs) const
{
    if (joinEUI.u < rhs.joinEUI.u)
        return true;
    if (joinEUI.u == rhs.joinEUI.u) {
        if (devEUI.u < rhs.devEUI.u)
            return true;
        if (devEUI.u == rhs.devEUI.u)
            return devNonce.u < rhs.devNonce.u;
    }
    return false;
}

bool JOIN_REQUEST_FRAME::operator>(const JOIN_REQUEST_FRAME &rhs) const
{
    if (joinEUI.u > rhs.joinEUI.u)
        return true;
    if (joinEUI.u == rhs.joinEUI.u) {
        if (devEUI.u > rhs.devEUI.u)
            return true;
        if (devEUI.u == rhs.devEUI.u)
            return devNonce.u > rhs.devNonce.u;
    }
    return false;
}

bool JOIN_REQUEST_FRAME::operator<=(const JOIN_REQUEST_FRAME &rhs) const
{
    if (joinEUI.u <= rhs.joinEUI.u)
        return true;
    if (joinEUI.u == rhs.joinEUI.u) {
        if (devEUI.u <= rhs.devEUI.u)
            return true;
        if (devEUI.u == rhs.devEUI.u)
            return devNonce.u <= rhs.devNonce.u;
    }
    return false;
}

bool JOIN_REQUEST_FRAME::operator>=(const JOIN_REQUEST_FRAME &rhs) const
{
    if (joinEUI.u >= rhs.joinEUI.u)
        return true;
    if (joinEUI.u == rhs.joinEUI.u) {
        if (devEUI.u >= rhs.devEUI.u)
            return true;
        if (devEUI.u == rhs.devEUI.u)
            return devNonce.u >= rhs.devNonce.u;
    }
    return false;
}

bool JOIN_REQUEST_FRAME::operator!=(const JOIN_REQUEST_FRAME &rhs) const
{
    return (joinEUI.u != rhs.joinEUI.u
        || devEUI.u != rhs.devEUI.u
        || devNonce.u != rhs.devNonce.u
    );
}

bool JOIN_ACCEPT_FRAME::operator==(
    const JOIN_ACCEPT_FRAME &rhs) const
{
    return memcmp(&hdr.joinNonce.c, &rhs.hdr.joinNonce.c, SIZE_JOIN_ACCEPT_FRAME) == 0;
}

bool JOIN_ACCEPT_FRAME::operator==(
    const JOIN_ACCEPT_FRAME_HEADER &rhs) const
{
    return memcmp(&hdr.joinNonce.c, &rhs.joinNonce.c, SIZE_JOIN_ACCEPT_FRAME) == 0;
}

bool MHDR::operator==(const MHDR &rhs) const
{
    return i == rhs.i;
}

bool MHDR::operator!=(const MHDR &rhs) const
{
    return i != rhs.i;
}

/**
 * @see file:///C:/Users/andrei/Downloads/tr005-lorawan-device-identification-qr-codes.pdf
 * The VendorID, 0xFFFF, is to be utilized prior to commercial production of a device.
 */
PROFILEID::PROFILEID()
    : u(0xffff0000)
{
}

PROFILEID::PROFILEID(
    const std::string &hex
)
    : u(0xffff0000)
{
    if (isHex(hex))
        u = (uint32_t) strtoull(hex.c_str(), nullptr, 16);
}

PROFILEID::PROFILEID(
    uint32_t value
)
    : u(value)
{

}

std::size_t PROFILEID::operator()(
    const PROFILEID &value
) const {
    return value.u;
}

bool PROFILEID::operator==(
    const PROFILEID &rhs
) const
{
    return rhs.u == u;
}

bool PROFILEID::operator<(
    const PROFILEID &rhs
) const
{
    return u < rhs.u;
}

bool PROFILEID::operator>(
    const PROFILEID &rhs
) const
{
    return u > rhs.u;
}

bool PROFILEID::operator!=(
    const PROFILEID &rhs
) const
{
    return u != rhs.u;
}

DataRate::DataRate()
{
    value.uplink = true;
    value.downlink = true;
    value.modulation = MODULATION_LORA;
    value.bandwidth = BANDWIDTH_INDEX_125KHZ;
    value.spreadingFactor = DRLORA_SF11;
    value.bps = 0;
}

DataRate::DataRate(
    const DataRate &val
)
{
    value.uplink = val.value.uplink;
    value.downlink = val.value.downlink;
    value.modulation = val.value.modulation;
    value.bandwidth = val.value.bandwidth;
    value.spreadingFactor = val.value.spreadingFactor;
    value.bps = val.value.bps;
}

DataRate::DataRate(
    const DATA_RATE &val
)
{
    value.uplink = val.uplink;
    value.downlink = val.downlink;
    value.modulation = val.modulation;
    value.bandwidth = val.bandwidth;
    value.spreadingFactor = val.spreadingFactor;
    value.bps = val.bps;
}

DataRate::DataRate(
    BANDWIDTH aBandwidth,
    SPREADING_FACTOR aSpreadingFactor
)
{
    value.uplink = true;
    value.downlink = true;
    value.modulation = MODULATION_LORA;
    value.bandwidth = aBandwidth;
    value.spreadingFactor = aSpreadingFactor;
    value.bps = 0;
}

DataRate::DataRate(
    uint32_t aBps
)
{
    value.uplink = true;
    value.downlink = true;
    value.modulation = MODULATION_FSK;
    value.bandwidth = BANDWIDTH_INDEX_125KHZ;
    value.spreadingFactor = DRLORA_SF5;
    value.bps = aBps;
}

void DataRate::setLora(
    BANDWIDTH aBandwidth,
    SPREADING_FACTOR aSpreadingFactor
)
{
    value.uplink = true;
    value.downlink = true;
    value.modulation = MODULATION_LORA;
    value.bandwidth = aBandwidth;
    value.spreadingFactor = aSpreadingFactor;
    value.bps = 0;
}

void DataRate::setFSK(
    uint32_t aBps
)
{
    value.uplink = true;
    value.downlink = true;
    value.modulation = MODULATION_FSK;
    value.bandwidth = BANDWIDTH_INDEX_125KHZ;
    value.spreadingFactor = DRLORA_SF5;
    value.bps = aBps;
}

std::string DataRate::toString() const
{
    std::stringstream ss;
    // std::boolalpha
    ss << "{\"uplink\": " << (value.uplink ? STR_TRUE_FALSE)
       << ", \"downlink\": " << (value.downlink ? STR_TRUE_FALSE)
       << ", \"modulation\": \"" << MODULATION2String(value.modulation)
       << "\", \"bandwidth\": " <<  BANDWIDTH2String(value.bandwidth)
       << ", \"spreadingFactor\": " << value.spreadingFactor
       << ", \"bps\": " <<  value.bps
       << "}";
    return ss.str();
}

static const char *getDeviceIdPropertyPtr(
    const DEVICE_ID &deviceId,
    enum NETWORK_IDENTITY_PROPERTY p
) {
    switch (p) {
        case NIP_ACTIVATION:
            return (char *) &deviceId.activation;
        case NIP_DEVICE_CLASS:
            return (char *) &deviceId.deviceclass;
        case NIP_DEVEUI:
            return (char *) &deviceId.devEUI.u;
        case NIP_NWKSKEY:
            return (char *) &deviceId.nwkSKey.u;
        case NIP_APPSKEY:
            return (char *) &deviceId.appSKey.u;
        case NIP_LORAWAN_VERSION:
            return (char *) &deviceId.version.c;
            // OTAA
        case NIP_APPEUI:
            return (char *) &deviceId.appEUI.u;
        case NIP_APPKEY:
            return (char *) &deviceId.appKey.u;
        case NIP_NWKKEY:
            return (char *) &deviceId.nwkKey.u;
        case NIP_DEVNONCE:
            return (char *) &deviceId.devNonce.c;
        case NIP_JOINNONCE:
            return (char *) &deviceId.joinNonce.c;
            // added for searching
        case NIP_DEVICENAME:
            return (char *) &deviceId.name.c;
        default:
            return (char *) &deviceId.activation;
    }
}

static const char *getNetworkIdentityPropertyPtr(
    const NETWORKIDENTITY &identity,
    enum NETWORK_IDENTITY_PROPERTY p
)
{
    switch (p) {
        case NIP_ADDRESS:
            return (char *) &identity.value.devaddr.u;
        case NIP_ACTIVATION:
            return (char *) &identity.value.devid.id.activation;
        case NIP_DEVICE_CLASS:
            return (char *) &identity.value.devid.id.deviceclass;
        case NIP_DEVEUI:
            return (char *) &identity.value.devid.id.devEUI.u;
        case NIP_NWKSKEY:
            return (char *) &identity.value.devid.id.nwkSKey.u;
        case NIP_APPSKEY:
            return (char *) &identity.value.devid.id.appSKey.u;
        case NIP_LORAWAN_VERSION:
            return (char *) &identity.value.devid.id.version.c;
        // OTAA
        case NIP_APPEUI:
            return (char *) &identity.value.devid.id.appEUI.u;
        case NIP_APPKEY:
            return (char *) &identity.value.devid.id.appKey.u;
        case NIP_NWKKEY:
            return (char *) &identity.value.devid.id.nwkKey.u;
        case NIP_DEVNONCE:
            return (char *) &identity.value.devid.id.devNonce.c;
        case NIP_JOINNONCE:
            return (char *) &identity.value.devid.id.joinNonce.c;
        // added for searching
        case NIP_DEVICENAME:
            return (char *) &identity.value.devid.id.name.c;
        default:
            return (char *) &identity.value.devaddr.u;
    }
}

#define NIP_SIZE_COUNT 14

static size_t NETWORK_IDENTITY_PROPERTY_SIZES[NIP_SIZE_COUNT] {
    0,
    sizeof(NETWORKIDENTITY::value.devaddr.u),
    sizeof(NETWORKIDENTITY::value.devid.id.activation),
    sizeof(NETWORKIDENTITY::value.devid.id.deviceclass),
    sizeof(NETWORKIDENTITY::value.devid.id.devEUI.u),
    sizeof(NETWORKIDENTITY::value.devid.id.nwkSKey.u),
    sizeof(NETWORKIDENTITY::value.devid.id.appSKey.u),
    sizeof(NETWORKIDENTITY::value.devid.id.version.c),
    // OTAA
    sizeof(NETWORKIDENTITY::value.devid.id.appEUI.u),
    sizeof(NETWORKIDENTITY::value.devid.id.appKey.u),
    sizeof(NETWORKIDENTITY::value.devid.id.nwkKey.u),
    sizeof(NETWORKIDENTITY::value.devid.id.devNonce.u),
    sizeof(NETWORKIDENTITY::value.devid.id.joinNonce.c),
    // added for searching
    sizeof(NETWORKIDENTITY::value.devid.id.name.c)
};

static size_t getNetworkIdentityPropertySize(
    enum NETWORK_IDENTITY_PROPERTY p
)
{
    if ((int) p < 0 || (int) p >= NIP_SIZE_COUNT)
        return 0;
    return NETWORK_IDENTITY_PROPERTY_SIZES[(int) p];
}


bool isIdentityFiltered(
    const NETWORKIDENTITY &identity,
    const NETWORK_IDENTITY_FILTER &filter
)
{
    auto sz = getNetworkIdentityPropertySize(filter.property);
    if (filter.length < sz)
        sz = filter.length;
    auto c = memcmp(
        getNetworkIdentityPropertyPtr(identity, filter.property),
        filter.filterData,
        sz
    );

    switch (filter.comparisonOperator) {
        case NICO_EQ:
            return c == 0;
        case NICO_GT:
            return c > 0;
        case NICO_LT:
            return c < 0;
        case NICO_GE:
            return c >= 0;
        case NICO_LE:
            return c <= 0;
        case NICO_NE:
            return c != 0;
        default:
            break;
    }
    return false;
}

bool isIdentityFiltered2(
    const DEVADDR &addr,
    const DEVICE_ID &deviceId,
    const NETWORK_IDENTITY_FILTER &filter
)
{
    auto sz = getNetworkIdentityPropertySize(filter.property);
    if (filter.length < sz)
        sz = filter.length;
    int c = 0;

    if (filter.property == NIP_ADDRESS)
        c = memcmp(&addr.u, &filter.filterData, sz);
    else
        c = memcmp(getDeviceIdPropertyPtr(deviceId, filter.property), filter.filterData,sz);

    switch (filter.comparisonOperator) {
        case NICO_EQ:
            return c == 0;
        case NICO_GT:
            return c > 0;
        case NICO_LT:
            return c < 0;
        case NICO_GE:
            return c >= 0;
        case NICO_LE:
            return c <= 0;
        case NICO_NE:
            return c != 0;
        default:
            break;
    }
    return false;
}

bool isIdentityFilteredV(
    const NETWORKIDENTITY &identity,
    const std::vector<NETWORK_IDENTITY_FILTER> &filters
)
{
    bool r = true;
    for (auto &f : filters) {
        bool c = isIdentityFiltered(identity, f);
        if (f.pre == NILPO_OR)
            r |= c;
        else
            r &= c;
        if (!r)
            break;
    }
    return r;
}

bool isIdentityFilteredV2(
    const DEVADDR &addr,
    const DEVICE_ID &deviceId,
    const std::vector<NETWORK_IDENTITY_FILTER> &filters
)
{
    bool r = true;
    for (auto &f : filters) {
        bool c = isIdentityFiltered2(addr, deviceId, f);
        if (f.pre == NILPO_OR)
            r |= c;
        else
            r &= c;
        if (!r)
            break;
    }
    return r;
}
