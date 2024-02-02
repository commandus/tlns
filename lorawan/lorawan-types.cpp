#include <cstring>
#include <sstream>
#include "lorawan-types.h"
#include "lorawan-string.h"
#include "lorawan-conv.h"
#include "lorawan-error.h"

const char *LIST_SEPARATOR = ",";

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

NETID::NETID() {
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
    std::stringstream ss(value.c_str());
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
            return 15;  // 15 unused bits, 3 bits type, 6 bits- identifier
        case 2:
            return 12;  // 12 unused bits, 3 bits type, 9 bits- identifier
        default:        // 3..7
            return 0;   // 0 unused bits, 3 bits type, 21 bits- identifier
    }
}

int NETID::getNetIdBitsCount() const
{
    switch (((NETID_TYPE*) this)->networkType) {
        case 0:
        case 1:
            return 6;    // 15 unused bits, 3 bits type, 6 bits- identifier
        case 2:
            return 9;    // 12 unused bits, 3 bits type, 9 bits- identifier
        default:    // 3..7
            return 21;   // 0 unused bits, 3 bits type, 21 bits- identifier
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

DEVICEID::DEVICEID() {
    this->activation = ABP;
    this->deviceclass = CLASS_A;
    memset(&this->devEUI.c, 0, sizeof(DEVEUI));
    memset(&this->nwkSKey.c, 0, sizeof(KEY128));
    memset(&this->appSKey.c, 0, sizeof(KEY128));
    
    memset(&this->appEUI.c, 0, sizeof(DEVEUI));
    memset(&this->nwkKey.c, 0, sizeof(KEY128));
    memset(&this->appKey.c, 0, sizeof(KEY128));
    this->devNonce.u = 0;
    memset(&this->joinNonce.c, 0, sizeof(JOINNONCE));
    memset(&this->name.c, 0, sizeof(DEVICENAME));
	version.major = 1;
}

DEVICEID::DEVICEID(
    const DEVEUI &deveui
)
    : devEUI(deveui)
{
    this->activation = ABP;
    this->deviceclass = CLASS_A;

    memset(&this->nwkSKey.c, 0, sizeof(KEY128));
    memset(&this->appSKey.c, 0, sizeof(KEY128));

    memset(&this->appEUI.c, 0, sizeof(DEVEUI));
    memset(&this->nwkKey.c, 0, sizeof(KEY128));
    memset(&this->appKey.c, 0, sizeof(KEY128));
    this->devNonce.u = 0;
    memset(&this->joinNonce.c, 0, sizeof(JOINNONCE));
    memset(&this->name.c, 0, sizeof(DEVICENAME));
    version.major = 1;
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
    const DEVICENAME name
) {
    this->activation = activation;
    this->deviceclass = deviceclass;
    memmove(&this->devEUI.c, &devEUI.c, sizeof(DEVEUI));
    memmove(&this->nwkSKey.c, &nwkSKey.c, sizeof(KEY128));
    memmove(&this->appSKey.c, &appSKey.c, sizeof(KEY128));
    this->version = version;
    memmove(&this->appEUI.c, &appEUI.c, sizeof(DEVEUI));
    memmove(&this->nwkKey.c, &nwkKey.c, sizeof(KEY128));
    memmove(&this->appKey.c, &appKey.c, sizeof(KEY128));
    this->devNonce = devNonce;
    memmove(&this->joinNonce.c, &joinNonce.c, sizeof(JOINNONCE));
    memmove(&this->name, &name, sizeof(DEVICENAME));
}

DEVICEID::DEVICEID(
    DEVICECLASS deviceclass,
    const DEVEUI &devEUI,
    const KEY128 &nwkSKey,
    const KEY128 &appSKey,
    LORAWAN_VERSION version,
    const DEVICENAME name
) {
    this->activation = ABP;
    this->deviceclass = deviceclass;
    memmove(&this->devEUI.c, &devEUI.c, sizeof(DEVEUI));
    memmove(&this->nwkSKey.c, &nwkSKey.c, sizeof(KEY128));
    memmove(&this->appSKey.c, &appSKey.c, sizeof(KEY128));
    this->version = version;
    memmove(&this->name, &name, sizeof(DEVICENAME));

    memset(&this->appEUI.c, 0, sizeof(DEVEUI));
    memset(&this->nwkKey.c, 0, sizeof(KEY128));
    memset(&this->appKey.c, 0, sizeof(KEY128));
    this->devNonce.u = 0;
    memset(&this->joinNonce.c, 0, sizeof(JOINNONCE));
}

DEVICEID::DEVICEID(
    uint64_t id
)
    : devEUI(id)
{
    this->activation = ABP;
    this->deviceclass = CLASS_A;
    memset(&this->devEUI.c, 0, sizeof(DEVEUI));
    memset(&this->nwkSKey.c, 0, sizeof(KEY128));
    memset(&this->appSKey.c, 0, sizeof(KEY128));

    memset(&this->appEUI.c, 0, sizeof(DEVEUI));
    memset(&this->nwkKey.c, 0, sizeof(KEY128));
    memset(&this->appKey.c, 0, sizeof(KEY128));
    this->devNonce.u = 0;
    memset(&this->joinNonce.c, 0, sizeof(JOINNONCE));
    memset(&this->name.c, 0, sizeof(DEVICENAME));
    version.major = 1;
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
	activation = value.activation;	///< activation type: ABP or OTAA
	deviceclass = value.deviceclass;
	memmove(&devEUI.c, &value.devEUI.c, sizeof(DEVEUI));
	memmove(&nwkSKey.c, &value.nwkSKey.c, sizeof(KEY128));
	memmove(&appSKey.c, &value.appSKey.c, sizeof(KEY128));
	version = value.version;		///< device LoraWAN version
    memmove(&appEUI.c, &value.appEUI.c, sizeof(DEVEUI));
    memmove(&appKey.c, &value.appKey.c, sizeof(KEY128));
    memmove(&nwkKey.c, &value.nwkKey.c, sizeof(KEY128));
    devNonce = value.devNonce;
    memmove(&joinNonce.c, &value.joinNonce.c, sizeof(JOINNONCE));
	memmove(&name, &value.name, sizeof(DEVICENAME));
	return *this;
}

DEVICEID& DEVICEID::operator=(const NETWORKIDENTITY& value)
{
    set(value.devid);
	return *this;
}

void DEVICEID::set(
	const DEVICEID &value
)
{
	memmove(&activation, &value.activation, sizeof(activation));
	memmove(&deviceclass, &value.deviceclass, sizeof(deviceclass));
	memmove(&devEUI.c, &value.devEUI.c, sizeof(DEVEUI));
	memmove(&nwkSKey.c, &value.nwkSKey.c, sizeof(KEY128));
	memmove(&appSKey.c, &value.appSKey.c, sizeof(KEY128));
	memmove(&version, &value.version.c, sizeof(LORAWAN_VERSION));
	memmove(&appEUI.c, &value.appEUI.c, sizeof(DEVEUI));
	memmove(&appKey.c, &value.appKey.c, sizeof(KEY128));
	memmove(&nwkKey.c, &value.nwkKey.c, sizeof(KEY128));
	devNonce = value.devNonce;
	memmove(&joinNonce.c, &value.joinNonce.c, sizeof(JOINNONCE));
	memmove(&name, &value.name, sizeof(DEVICENAME));
}

void DEVICEID::setEUIString
(
	const std::string &value
)
{
	string2DEVEUI(devEUI, value);
}

void DEVICEID::setNwkSKeyString
(
	const std::string &value
)
{
	string2KEY(nwkSKey, value);
}

void DEVICEID::setAppSKeyString(
	const std::string &value
)
{
	string2KEY(appSKey, value);
}

void DEVICEID::setName(
	const std::string &value
)
{
	string2DEVICENAME(name, value.c_str());
}

void DEVICEID::setClass(
	const DEVICECLASS &value
)
{
	deviceclass = value;
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
        << activation2string(activation)
        << LIST_SEPARATOR << deviceclass2string(deviceclass)
        << LIST_SEPARATOR << DEVEUI2string(devEUI)
        << LIST_SEPARATOR << KEY2string(nwkSKey)
        << LIST_SEPARATOR << KEY2string(appSKey)
        << LIST_SEPARATOR << LORAWAN_VERSION2string(version)
        << LIST_SEPARATOR << DEVEUI2string(appEUI)
        << LIST_SEPARATOR << KEY2string(appKey)
        << LIST_SEPARATOR << KEY2string(nwkKey)
        << LIST_SEPARATOR << DEVNONCE2string(devNonce)
        << LIST_SEPARATOR << JOINNONCE2string(joinNonce)
        << LIST_SEPARATOR << DEVICENAME2string(name);
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
        ss << "\"addr\":\"" << DEVADDR2string(addr) << "\",";
    ss << "\"activation\":\"" << activation2string(activation)
       << "\",\"class\":\"" << deviceclass2string(deviceclass)
       << "\",\"deveui\":\"" << DEVEUI2string(devEUI)
       << "\",\"nwkSKey\":\"" << KEY2string(nwkSKey)
       << "\",\"appSKey\":\"" << KEY2string(appSKey)
       << "\",\"version\":\"" << LORAWAN_VERSION2string(version)

       << "\",\"appeui\":\"" << DEVEUI2string(appEUI)
       << "\",\"appKey\":\"" << KEY2string(appKey)
       << "\",\"nwkKey\":\"" << KEY2string(nwkKey)
       << "\",\"devNonce\":\"" << DEVNONCE2string(devNonce)
       << "\",\"joinNonce\":\"" << JOINNONCE2string(joinNonce)

       << "\",\"name\":\"" << name.toString()
       << "\"}";
    return ss.str();
}

void DEVICEID::setProperties
(
	std::map<std::string, std::string> &retval
)
{
	retval["activation"] = activation2string(activation);
	retval["class"] = deviceclass2string(deviceclass);
	retval["deveui"] = DEVEUI2string(devEUI);
    retval["appeui"] = DEVEUI2string(appEUI);
    retval["appKey"] = KEY2string(appKey);
    retval["nwkKey"] = KEY2string(nwkKey);
    retval["devNonce"] = DEVNONCE2string(devNonce);
    retval["joinNonce"] = JOINNONCE2string(joinNonce);
	retval["name"] = DEVICENAME2string(name);
	retval["version"] = LORAWAN_VERSION2string(version);
}

bool DEVICEID::empty() const
{
    return devEUI.u == 0;
}

NETWORKIDENTITY::NETWORKIDENTITY()
{
}

NETWORKIDENTITY::NETWORKIDENTITY(
    const DEVADDR &a,
    const DEVICEID &id
)
{
    devaddr.u = a.u;
    devid = id;
}

NETWORKIDENTITY::NETWORKIDENTITY(
    const NETWORKIDENTITY &id
)
    : devaddr(id.devaddr), devid(id.devid)
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
    set(id.devaddr, id.devid);
}

void NETWORKIDENTITY::set(
    const DEVADDR &addr,
    const DEVICEID &value
)
{
    devaddr.u = addr.u;
    devid.set(value);
}

std::string NETWORKIDENTITY::toString() const 
{
    return devid.toString(devaddr);
}

std::string NETWORKIDENTITY::toJsonString() const
{
	return devid.toJsonString(devaddr);
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
    return memcmp(&hdr, &rhs.hdr, SIZE_JOIN_ACCEPT_FRAME) == 0;
}

bool JOIN_ACCEPT_FRAME::operator==(
    const JOIN_ACCEPT_FRAME_HEADER &rhs) const
{
    return memcmp(&hdr, &rhs, SIZE_JOIN_ACCEPT_FRAME) == 0;
}

bool MHDR::operator==(const MHDR &rhs) const
{
    return i == rhs.i;
}

bool MHDR::operator!=(const MHDR &rhs) const
{
    return i != rhs.i;
}
