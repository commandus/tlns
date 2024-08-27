#include <cstring>

#include "lorawan/lorawan-conv.h"
#include "service-serialization.h"
#include "lorawan/helper/ip-address.h"

#ifdef ESP_PLATFORM
#include "lwip/ip6_addr.h"
#endif

ServiceMessage::ServiceMessage()
    : tag(0), code(0), accessCode(0)
{

}

ServiceMessage::ServiceMessage(
    char aTag,
    int32_t aCode,
    uint64_t aAccessCode
)
    : tag(aTag), code(aCode), accessCode(aAccessCode)
{

}

ServiceMessage::ServiceMessage(
    const unsigned char *buf,
    size_t sz
)
{
    if (sz >= SIZE_SERVICE_MESSAGE) {
        memmove(&tag, &buf[0], sizeof(tag));         // 1
        memmove(&code, &buf[1], sizeof(code));       // 4
        memmove(&accessCode, &buf[5], sizeof(accessCode)); // 8
    }   // 13 bytes
}

ServiceMessage::~ServiceMessage()
{

}

void ServiceMessage::ntoh()
{
    code = NTOH4(code);
    accessCode = NTOH8(accessCode);
}

size_t ServiceMessage::serialize(
    unsigned char *retBuf
) const
{
    if (retBuf) {
        memmove(&retBuf[0], &tag, sizeof(tag));         // 1
        memmove(&retBuf[1], &code, sizeof(code));       // 4
        memmove(&retBuf[5], &accessCode, sizeof(accessCode)); // 8
    }
    return SIZE_SERVICE_MESSAGE;                        // 13
}

std::string ServiceMessage::toJsonString() const
{
    return "";
}

struct in_addr_4 {
    union {
        struct {
            u_char s_b1;
            u_char s_b2;
            u_char s_b3;
            u_char s_b4;
        } S_un_b;
        u_long S_addr;
    } S_un;
};


/**
 * Serialize Internet address v4, v6
 * @param retBuf return buffer
 * @param addr address to serialize
 * @return 0 (unknown family), 7 (IPv4, 19(IPv6)
 */
size_t serializeSocketAddress(
    unsigned char *retBuf,
    const struct sockaddr *addr
)
{
    size_t r = 0;
    if (!retBuf) {
        switch (addr->sa_family) {
            case AF_INET:
                return 7;
            case AF_INET6:
                return 19;
        }
        return 0;
    }
    switch (addr->sa_family) {
        case AF_INET:
            retBuf[0] = AF_INET;
            {
                auto *addrIn = (struct sockaddr_in *) addr;
                retBuf[1] = *(unsigned char *) &addrIn->sin_port;
                retBuf[2] = * ((unsigned char *) &addrIn->sin_port + 1);
#if defined(_MSC_VER) || defined(__MINGW32__)
                retBuf[3] = addrIn->sin_addr.S_un.S_un_b.s_b1;
                retBuf[4] = addrIn->sin_addr.S_un.S_un_b.s_b2;
                retBuf[5] = addrIn->sin_addr.S_un.S_un_b.s_b3;
                retBuf[6] = addrIn->sin_addr.S_un.S_un_b.s_b4;
#else
                auto *p4 = (struct in_addr_4 *) &addrIn->sin_addr.s_addr;
                retBuf[3] = p4->S_un.S_un_b.s_b1;
                retBuf[4] = p4->S_un.S_un_b.s_b2;
                retBuf[5] = p4->S_un.S_un_b.s_b3;
                retBuf[6] = p4->S_un.S_un_b.s_b4;
#endif
            }
            r = 7;
            break;
        case AF_INET6:
            retBuf[0] = AF_INET6;
            {
                auto *addrIn = (struct sockaddr_in6 *) addr;
                retBuf[1] = *(unsigned char *) &addrIn->sin6_port;
                retBuf[2] = * ((unsigned char *) &addrIn->sin6_port + 1);

#if defined(_MSC_VER) || defined(__MINGW32__)
                retBuf[3] = addrIn->sin6_addr.u.Byte[0];
                retBuf[4] = addrIn->sin6_addr.u.Byte[1];
                retBuf[5] = addrIn->sin6_addr.u.Byte[2];
                retBuf[6] = addrIn->sin6_addr.u.Byte[3];

                retBuf[7] = addrIn->sin6_addr.u.Byte[4];
                retBuf[8] = addrIn->sin6_addr.u.Byte[5];
                retBuf[9] = addrIn->sin6_addr.u.Byte[6];
                retBuf[10] = addrIn->sin6_addr.u.Byte[7];

                retBuf[11] = addrIn->sin6_addr.u.Byte[8];
                retBuf[12] = addrIn->sin6_addr.u.Byte[9];
                retBuf[13] = addrIn->sin6_addr.u.Byte[10];
                retBuf[14] = addrIn->sin6_addr.u.Byte[11];

                retBuf[15] = addrIn->sin6_addr.u.Byte[12];
                retBuf[16] = addrIn->sin6_addr.u.Byte[13];
                retBuf[17] = addrIn->sin6_addr.u.Byte[14];
                retBuf[18] = addrIn->sin6_addr.u.Byte[15];
#else
#ifdef ESP_PLATFORM
                retBuf[3] = addrIn->sin6_addr.un.u8_addr[0];
                retBuf[4] = addrIn->sin6_addr.un.u8_addr[1];
                retBuf[5] = addrIn->sin6_addr.un.u8_addr[2];
                retBuf[6] = addrIn->sin6_addr.un.u8_addr[3];

                retBuf[7] = addrIn->sin6_addr.un.u8_addr[4];
                retBuf[8] = addrIn->sin6_addr.un.u8_addr[5];
                retBuf[9] = addrIn->sin6_addr.un.u8_addr[6];
                retBuf[10] = addrIn->sin6_addr.un.u8_addr[7];

                retBuf[11] = addrIn->sin6_addr.un.u8_addr[8];
                retBuf[12] = addrIn->sin6_addr.un.u8_addr[9];
                retBuf[13] = addrIn->sin6_addr.un.u8_addr[10];
                retBuf[14] = addrIn->sin6_addr.un.u8_addr[11];

                retBuf[15] = addrIn->sin6_addr.un.u8_addr[12];
                retBuf[16] = addrIn->sin6_addr.un.u8_addr[13];
                retBuf[17] = addrIn->sin6_addr.un.u8_addr[14];
                retBuf[18] = addrIn->sin6_addr.un.u8_addr[15];
#else
                retBuf[3] = addrIn->sin6_addr.__in6_u.__u6_addr8[0];
                retBuf[4] = addrIn->sin6_addr.__in6_u.__u6_addr8[1];
                retBuf[5] = addrIn->sin6_addr.__in6_u.__u6_addr8[2];
                retBuf[6] = addrIn->sin6_addr.__in6_u.__u6_addr8[3];

                retBuf[7] = addrIn->sin6_addr.__in6_u.__u6_addr8[4];
                retBuf[8] = addrIn->sin6_addr.__in6_u.__u6_addr8[5];
                retBuf[9] = addrIn->sin6_addr.__in6_u.__u6_addr8[6];
                retBuf[10] = addrIn->sin6_addr.__in6_u.__u6_addr8[7];

                retBuf[11] = addrIn->sin6_addr.__in6_u.__u6_addr8[8];
                retBuf[12] = addrIn->sin6_addr.__in6_u.__u6_addr8[9];
                retBuf[13] = addrIn->sin6_addr.__in6_u.__u6_addr8[10];
                retBuf[14] = addrIn->sin6_addr.__in6_u.__u6_addr8[11];

                retBuf[15] = addrIn->sin6_addr.__in6_u.__u6_addr8[12];
                retBuf[16] = addrIn->sin6_addr.__in6_u.__u6_addr8[13];
                retBuf[17] = addrIn->sin6_addr.__in6_u.__u6_addr8[14];
                retBuf[18] = addrIn->sin6_addr.__in6_u.__u6_addr8[15];
#endif
#endif
            }
            r = 19;
            break;
        default:
            break;
    }
    return r;
}

size_t deserializeSocketAddress(
    struct sockaddr *addr,
    const unsigned char *retBuf,
    size_t sz
)
{
    size_t r = 0;
    if (sz < 1)
        return r;
    switch (retBuf[0]) {
        case AF_INET:
            if (sz < 7)
                return 0;
            addr->sa_family = AF_INET;
            {
                auto *addrIn = (struct sockaddr_in *) addr;
                *(unsigned char *) &addrIn->sin_port = retBuf[1];
                *((unsigned char *) &addrIn->sin_port + 1) = retBuf[2];

#if defined(_MSC_VER) || defined(__MINGW32__)
                addrIn->sin_addr.S_un.S_un_b.s_b1 = retBuf[3];
                addrIn->sin_addr.S_un.S_un_b.s_b2 = retBuf[4];
                addrIn->sin_addr.S_un.S_un_b.s_b3 = retBuf[5];
                addrIn->sin_addr.S_un.S_un_b.s_b4 = retBuf[6];
#else
                auto *p4 = (struct in_addr_4 *) &addrIn->sin_addr.s_addr;
                p4->S_un.S_un_b.s_b1 = retBuf[3];
                p4->S_un.S_un_b.s_b2 = retBuf[4];
                p4->S_un.S_un_b.s_b3 = retBuf[5];
                p4->S_un.S_un_b.s_b4 = retBuf[6];
#endif
            }
            r = 7;
            break;
        case AF_INET6:
            if (sz < 19)
                return 0;
            addr->sa_family = AF_INET6;
            {
                auto *addrIn = (struct sockaddr_in6 *) addr;
                *(unsigned char *) &addrIn->sin6_port = retBuf[1];
                *((unsigned char *) &addrIn->sin6_port + 1) = retBuf[2];
#if defined(_MSC_VER) || defined(__MINGW32__)
                addrIn->sin6_addr.u.Byte[0] = retBuf[3];
                addrIn->sin6_addr.u.Byte[1] = retBuf[4];
                addrIn->sin6_addr.u.Byte[2] = retBuf[5];
                addrIn->sin6_addr.u.Byte[3] = retBuf[6];

                addrIn->sin6_addr.u.Byte[4] = retBuf[7];
                addrIn->sin6_addr.u.Byte[5] = retBuf[8];
                addrIn->sin6_addr.u.Byte[6] = retBuf[9];
                addrIn->sin6_addr.u.Byte[7] = retBuf[10];

                addrIn->sin6_addr.u.Byte[8] = retBuf[11];
                addrIn->sin6_addr.u.Byte[9] = retBuf[12];
                addrIn->sin6_addr.u.Byte[10] = retBuf[13];
                addrIn->sin6_addr.u.Byte[11] = retBuf[14];

                addrIn->sin6_addr.u.Byte[12] = retBuf[15];
                addrIn->sin6_addr.u.Byte[13] = retBuf[16];
                addrIn->sin6_addr.u.Byte[14] = retBuf[17];
                addrIn->sin6_addr.u.Byte[15] = retBuf[18];
#else
#ifdef ESP_PLATFORM
                addrIn->sin6_addr.un.u8_addr[0] = retBuf[3];
                addrIn->sin6_addr.un.u8_addr[1] = retBuf[4];
                addrIn->sin6_addr.un.u8_addr[2] = retBuf[5];
                addrIn->sin6_addr.un.u8_addr[3] = retBuf[6];

                addrIn->sin6_addr.un.u8_addr[4] = retBuf[7];
                addrIn->sin6_addr.un.u8_addr[5] = retBuf[8];
                addrIn->sin6_addr.un.u8_addr[6] = retBuf[9];
                addrIn->sin6_addr.un.u8_addr[7] = retBuf[10];

                addrIn->sin6_addr.un.u8_addr[8] = retBuf[11];
                addrIn->sin6_addr.un.u8_addr[9] = retBuf[12];
                addrIn->sin6_addr.un.u8_addr[10] = retBuf[13];
                addrIn->sin6_addr.un.u8_addr[11] = retBuf[14];

                addrIn->sin6_addr.un.u8_addr[12] = retBuf[15];
                addrIn->sin6_addr.un.u8_addr[13] = retBuf[16];
                addrIn->sin6_addr.un.u8_addr[14] = retBuf[17];
                addrIn->sin6_addr.un.u8_addr[15] = retBuf[18];
#else
                addrIn->sin6_addr.__in6_u.__u6_addr8[0] = retBuf[3];
                addrIn->sin6_addr.__in6_u.__u6_addr8[1] = retBuf[4];
                addrIn->sin6_addr.__in6_u.__u6_addr8[2] = retBuf[5];
                addrIn->sin6_addr.__in6_u.__u6_addr8[3] = retBuf[6];

                addrIn->sin6_addr.__in6_u.__u6_addr8[4] = retBuf[7];
                addrIn->sin6_addr.__in6_u.__u6_addr8[5] = retBuf[8];
                addrIn->sin6_addr.__in6_u.__u6_addr8[6] = retBuf[9];
                addrIn->sin6_addr.__in6_u.__u6_addr8[7] = retBuf[10];

                addrIn->sin6_addr.__in6_u.__u6_addr8[8] = retBuf[11];
                addrIn->sin6_addr.__in6_u.__u6_addr8[9] = retBuf[12];
                addrIn->sin6_addr.__in6_u.__u6_addr8[10] = retBuf[13];
                addrIn->sin6_addr.__in6_u.__u6_addr8[11] = retBuf[14];

                addrIn->sin6_addr.__in6_u.__u6_addr8[12] = retBuf[15];
                addrIn->sin6_addr.__in6_u.__u6_addr8[13] = retBuf[16];
                addrIn->sin6_addr.__in6_u.__u6_addr8[14] = retBuf[17];
                addrIn->sin6_addr.__in6_u.__u6_addr8[15] = retBuf[18];
#endif
#endif
            }
            r = 19;
            break;
        default:
            break;
    }
    return r;
}
