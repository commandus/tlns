#include "lorawan-conv.h"

bool isDEVADDREmpty(const DEVADDR &addr)
{
    return *((uint32_t *) &addr) == 0;
}

bool isDEVEUIEmpty(const DEVEUI &eui)
{
    return *((uint64_t *) &eui) == 0;
}

int hasFPort(
    const void *value,
    size_t size
)
{
    if (!value || size <= SIZE_RFM_HEADER)
        return -1;
    if (size < SIZE_RFM_HEADER + ((const RFM_HEADER*) value)->fctrl.f.foptslen)
        return -1;
    return ((uint8_t*) value)[SIZE_RFM_HEADER + ((const RFM_HEADER*) value)->fctrl.f.foptslen];
}

char* hasPayload(
    const void *value,
    size_t size
)
{
    if (!value || size <= SIZE_RFM_HEADER)
        return nullptr;
    if (size > SIZE_RFM_HEADER + ((const RFM_HEADER*) value)->fctrl.f.foptslen + 1)
        return ((char *) value) + SIZE_RFM_HEADER + ((const RFM_HEADER*) value)->fctrl.f.foptslen + 1;
    return nullptr;
}

uint8_t getFPort(
    const void *value
)
{
    return ((uint8_t*) value)[SIZE_RFM_HEADER + ((const RFM_HEADER*) value)->fctrl.f.foptslen];
}

uint32_t DEVADDR2int(
	const DEVADDR &value
)
{
    uint32_t retval;
    *((uint32_t*) &retval) = NTOH4(*((uint32_t*) &value));
    return retval;
}

void int2DEVADDR(
	DEVADDR &retval,
	uint32_t value
)
{
	// *((uint32_t*) &retval) = NTOH4(value);
	*((uint32_t*) &retval) = value;
}

uint32_t NETID2int(
	const NETID &value
) {
	return value.c[0] + (value.c[1] << 8) + (value.c[2] << 16);
}

void int2NETID(
	NETID &retval,
	uint32_t value
)
{
	retval.c[0] = value & 0xff;
	retval.c[1] = (value >> 8) & 0xff;
	retval.c[2] = (value >> 16) & 0xff;
}

uint32_t JOINNONCE2int(
	const JOINNONCE &value
) {
	return value.c[0] + (value.c[1] << 8) + (value.c[2] << 16);
}

int FREQUENCY2int(
	const FREQUENCY &frequency
) {
	return frequency[0] + (frequency[1] << 8) + (frequency[2] << 16);
}

void int2JOINNONCE(
    JOINNONCE &retVal,
    int value
)
{
    uint32_t r = NTOH4(value);
    retVal.c[0] = r & 0xff;
    retVal.c[1] = (r >> 8) & 0xff;
    retVal.c[2] = (r >> 16) & 0xff;
}

void int2APPNONCE(
    APPNONCE& retVal,
    int value
)
{
    uint32_t r = NTOH4(value);
    retVal.c[0] = r & 0xff;
    retVal.c[1] = (r >> 8) & 0xff;
    retVal.c[2] = (r >> 16) & 0xff;
}

#ifdef IS_LITTLE_ENDIAN

void ntoh_DEVADDR(
    DEVADDR &value
)
{
    *((uint32_t*) &value.u) = NTOH4(*((uint32_t*) &value.u));
}

void ntoh_DEVEUI(
    DEVEUI &value
)
{
    *((uint64_t*) &value.u) = NTOH8(*((uint64_t*) &value.u));
}

void ntoh_SEMTECH_PREFIX_GW(
    SEMTECH_PREFIX_GW &value
)
{
    *((uint16_t*) &value.token) = NTOH2(*((uint16_t*) &value.token));
    *((uint64_t*) &value.mac.u) = NTOH8(*((uint64_t*) &value.mac.u));
}

void ntoh_RFM_HEADER(
    RFM_HEADER *value
)
{
    ntoh_DEVADDR(value->devaddr);
    value->fcnt = NTOH2(value->fcnt);	// frame counter 0..65535
}

#endif

BANDWIDTH int2BANDWIDTH(int value)
{
    if (value == 7)
        return BANDWIDTH_INDEX_7KHZ;
    if (value == 10)
        return BANDWIDTH_INDEX_10KHZ;
    if (value == 15)
        return BANDWIDTH_INDEX_15KHZ;
    if (value == 20)
        return BANDWIDTH_INDEX_20KHZ;
    if (value == 31)
        return BANDWIDTH_INDEX_31KHZ;
    if (value == 41)
        return BANDWIDTH_INDEX_41KHZ;
    if (value == 62)
        return BANDWIDTH_INDEX_62KHZ;
    if (value == 125)
        return BANDWIDTH_INDEX_125KHZ;
    if (value == 250)
        return BANDWIDTH_INDEX_250KHZ;
    if (value == 500)
        return BANDWIDTH_INDEX_500KHZ;
    return BANDWIDTH_INDEX_7KHZ;
}

/**
 * @param value 7..500 or 7000..
 */
BANDWIDTH double2BANDWIDTH(double value)
{
    if (value <= 500)
        return int2BANDWIDTH((int) value);
    else {
        if (value >= 7000)
            return BANDWIDTH_INDEX_7KHZ;
        if (value >= 10000)
            return BANDWIDTH_INDEX_10KHZ;
        if (value >= 15000)
            return BANDWIDTH_INDEX_15KHZ;
        if (value >= 20000)
            return BANDWIDTH_INDEX_20KHZ;
        if (value >= 31000)
            return BANDWIDTH_INDEX_31KHZ;
        if (value >= 41000)
            return BANDWIDTH_INDEX_41KHZ;
        if (value >= 62000)
            return BANDWIDTH_INDEX_62KHZ;
        if (value >= 125000)
            return BANDWIDTH_INDEX_125KHZ;
        if (value >= 250000)
            return BANDWIDTH_INDEX_250KHZ;
        if (value >= 500000)
            return BANDWIDTH_INDEX_500KHZ;
        return BANDWIDTH_INDEX_7KHZ;
    }
}
