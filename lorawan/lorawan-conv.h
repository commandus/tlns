#ifndef LORAWAN_CONV_H_
#define LORAWAN_CONV_H_	1

#include "lorawan-types.h"

#ifdef __ANDROID__
#include <endian.h>
#endif

/*
 * Conversion functions from one type to another except string.
 * String conversion defined in the @see lorawan-string.h
 */

#ifdef ESP_PLATFORM
    #include <arpa/inet.h>
#endif
#if defined(_MSC_VER) || defined(__MINGW32__)
    #include <WinSock2.h>
#endif
#if defined(_MSC_VER)
    #define SWAP_BYTES_2(x) htons(x)
    #define SWAP_BYTES_4(x) htonl(x)
    #define SWAP_BYTES_8(x) htonll(x)
#else
    #if defined(__MINGW32__)
        #define SWAP_BYTES_2(x) htons(x)
        #define SWAP_BYTES_4(x) htonl(x)
        #define SWAP_BYTES_8(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
    #else
        #ifdef ESP_PLATFORM
            #define SWAP_BYTES_2(x) lwip_htons(x)
            #define SWAP_BYTES_4(x) lwip_htonl(x)
            #define SWAP_BYTES_8(x) ((((uint64_t) lwip_htonl(x)) << 32) + lwip_htonl((uint64_t)(x) >> 32))
        #else
            #define SWAP_BYTES_2(x) be16toh(x)
            #define SWAP_BYTES_4(x) be32toh(x)
            #define SWAP_BYTES_8(x) be64toh(x)
        #endif
    #endif
#endif


#if defined(_MSC_VER)
// #if BYTE_ORDER == BIG_ENDIAN does work on Windows platform because no endian.h header file
// @see https://stackoverflow.com/questions/2100331/macro-definition-to-determine-big-endian-or-little-endian-machine
#define LITTLE_ENDIAN 0x41424344UL
#define BIG_ENDIAN    0x44434241UL
#define PDP_ENDIAN    0x42414443UL
#define ENDIAN_ORDER  ('ABCD')
#define IS_BIG_ENDIAN (ENDIAN_ORDER == BIG_ENDIAN)
#else
#define IS_BIG_ENDIAN BYTE_ORDER == BIG_ENDIAN
#endif

#if IS_BIG_ENDIAN
    #define NTOH2(x) (x)
    #define NTOH4(x) (x)
    #define NTOH8(x) (x)
    #define HTON2(x) (x)
    #define HTON4(x) (x)
    #define HTON8(x) (x)
#else
    #define NTOH2(x) SWAP_BYTES_2(x)
    #define NTOH4(x) SWAP_BYTES_4(x)
    #define NTOH8(x) SWAP_BYTES_8(x)
    #define HTON2(x) SWAP_BYTES_2(x)
    #define HTON4(x) SWAP_BYTES_4(x)
    #define HTON8(x) SWAP_BYTES_92(x)
#endif

bool isDEVADDREmpty(const DEVADDR &addr);
bool isDEVEUIEmpty(const DEVEUI &eui);
/**
 * Return 0..255 port number, -1 if not
 * @param value RFM
 * @param size data size
 * @return 0..255 port number, -1 if not
 */
int hasFPort(const void *value, size_t size);
uint8_t getFPort(const void *value);

/**
 * Return pointer to the payload or NULL if no payload,
 * @param value buffer
 * @param size buffer size
 * @return pointer to the payload or NULL if no payload,
 */
char* hasPayload(const void *value, size_t size);

/**
 * Return payload size (except FOpts)
 * @param value buffer with MIC
 * @param size buffer size
 * @return 0 if not
 */
uint8_t payloadSize(
    const void *value,
    size_t size
);

uint32_t DEVADDR2int(const DEVADDR &value);
void int2DEVADDR(DEVADDR &retval, uint32_t value);

uint32_t NETID2int(const NETID &value);
void int2NETID(NETID &retval, uint32_t value);

uint32_t JOINNONCE2int(const JOINNONCE &value);

int FREQUENCY2int(const FREQUENCY &frequency);

void int2JOINNONCE(JOINNONCE &retVal, int value);

void int2APPNONCE(APPNONCE& retVal, int value);

#ifdef IS_LITTLE_ENDIAN
void ntoh_DEVADDR(DEVADDR &value);
void ntoh_DEVEUI(DEVEUI &value);
void ntoh_DEVNONCE(DEVNONCE &value);
void ntoh_RFM_HEADER(RFM_HEADER *value);
void ntoh_SEMTECH_PREFIX_GW(SEMTECH_PREFIX_GW &value);
void ntoh_JOIN_REQUEST_HEADER(JOIN_REQUEST_HEADER &value);
void ntoh_JOIN_REQUEST_FRAME(JOIN_REQUEST_FRAME &value);
void ntoh_JOIN_ACCEPT_FRAME_HEADER(JOIN_ACCEPT_FRAME_HEADER &value);
void ntoh_JOIN_ACCEPT_FRAME(JOIN_ACCEPT_FRAME &value);
void ntoh_RFM_HEADER(RFM_HEADER &value);
void ntoh_FHDR(FHDR &value);

#else
#define ntoh_DEVADDR(a) {}
#define ntoh_DEVEUI(e) {}
#define ntoh_DEVNONCE(v) {}
#define ntoh_RFM_HEADER(v) {}
#define ntoh_SEMTECH_PREFIX_GW(v) {}
#define ntoh_JOIN_REQUEST_HEADER(v) {}
#define ntoh_JOIN_REQUEST_FRAME(v) {}
#define ntoh_JOIN_ACCEPT_FRAME(v) {}
#define ntoh_RFM_HEADER(v) {}
#define ntoh_FHDR(v) {}
#endif

BANDWIDTH int2BANDWIDTH(int value);

BANDWIDTH double2BANDWIDTH(double value);

void applyNetworkByteOrder(void *buf, size_t size);
#define applyHostByteOrder(b, s) applyNetworkByteOrder(b, s)

#endif
