#include "udp-listener.h"

#include <iostream>

#ifdef ESP_PLATFORM
#include "platform-defs.h"
#include "esp_log.h"
#endif

#ifdef ENABLE_DEBUG
#include <iostream>
#endif
#if defined(_MSC_VER) || defined(__MINGW32__)
#include <iostream>
    #include <ws2tcpip.h>
    #include <io.h>
    #define write _write
    #define close closesocket
#else
    #define INVALID_SOCKET (-1)
    #ifdef ESP_PLATFORM
        #include "esp_log.h"
        #include "esp_netif.h"

        #include "lwip/err.h"
        #include "lwip/sockets.h"
        #include "lwip/sys.h"
        #include <lwip/netdb.h>
    #else
        #include <netinet/in.h>
        #include <cstring>
        #include <unistd.h>
    #endif
#endif

#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-msg.h"
#include "lorawan/helper/ip-address.h"

#define DEF_KEEPALIVE_SECS 60

#ifdef _MSC_VER
#define SOCKET_ERRNO WSAGetLastError()
#define SOCKET_ERROR_TIMEOUT WSAETIMEDOUT
#else
#define SOCKET_ERRNO errno
#define SOCKET_ERROR_TIMEOUT EAGAIN
#endif

/**
 * @see https://habr.com/ru/post/340758/
 * @see https://github.com/Mityuha/grpc_async/blob/master/grpc_async_server.cc
 */
UDPListener::UDPListener(
    IdentitySerialization *aIdentitySerialization,
    GatewaySerialization *aSerializationWrapper
)
    : StorageListener(aIdentitySerialization, aSerializationWrapper), destAddr({}), log(nullptr), verbose(0), status(CODE_OK)
{
}

void UDPListener::setLog(
    int aVerbose,
    Log *aLog
)
{
    verbose = aVerbose;
    log = aLog;
}

// http://stackoverflow.com/questions/25615340/closing-libuv-handles-correctly
void UDPListener::stop()
{
    status = ERR_CODE_STOPPED;
}

void UDPListener::setAddress(
    const std::string &host,
    uint16_t port
)
{
    int addr_family = isAddrStringIPv6(host.c_str()) ? AF_INET6 : AF_INET;
    if (addr_family == AF_INET) {
        auto *destAddrIP4 = (struct sockaddr_in *) &destAddr;
        destAddrIP4->sin_addr.s_addr = htonl(INADDR_ANY);
        destAddrIP4->sin_family = AF_INET;
        destAddrIP4->sin_port = htons(port);
    } else {
        auto *destAddrIP6 = (struct sockaddr_in6 *) &destAddr;
        memset(&destAddrIP6->sin6_addr, 0, sizeof(destAddrIP6->sin6_addr));
        destAddrIP6->sin6_family = AF_INET6;
        destAddrIP6->sin6_port = htons(port);
    }
}

void UDPListener::setAddress(
    uint32_t &ipv4,
    uint16_t port
)
{
    auto *a = (struct sockaddr_in *) &destAddr;
    a->sin_family = AF_INET;
    a->sin_addr.s_addr = ipv4;
    a->sin_port = htons(port);
}

int UDPListener::run()
{
    unsigned char rxBuf[307];

    int proto = isIPv6(&destAddr) ? IPPROTO_IPV6 : IPPROTO_IP;
    int af = isIPv6(&destAddr) ? AF_INET6 : AF_INET;

    int r = CODE_OK;
    while (status != ERR_CODE_STOPPED) {
        SOCKET sock = socket(af, SOCK_DGRAM, proto);
        if (sock == INVALID_SOCKET) {
            if (log) {
                log->strm(LOG_ERR) << ERR_SOCKET_CREATE
                    << MSG_SPACE << ERR_MESSAGE << SOCKET_ERRNO;
                log->flush();
            }
            r = ERR_CODE_SOCKET_CREATE;
            break;
        }
#ifdef _MSC_VER
        if (log && verbose > 1) {
            log->strm(LOG_INFO) << "Socket created ";
            log->flush();
        }
#endif
        int enable = 1;
        if (setsockopt(sock, IPPROTO_IP, IP_PKTINFO, (const char*) &enable, sizeof(enable))) {
            if (log) {
                log->strm(LOG_ERR) << ERR_SOCKET_SET
                    << MSG_SPACE << ERR_MESSAGE << SOCKET_ERRNO;
                log->flush();
            }
        }

        if (af == AF_INET6) {
            // Note that by default IPV6 binds to both protocols, it must be disabled
            // if both protocols used at the same time (used in CI)
            int opt = 1;
            setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*) &opt, sizeof(opt));
            setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char*) &opt, sizeof(opt));
        }

        // Set timeout
#ifdef _MSC_VER
        DWORD timeout = 1000;   // ms
#else
        struct timeval timeout { 1, 0 };
#endif
        if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof timeout)) {
            if (log) {
                log->strm(LOG_ERR) << ERR_SOCKET_SET
                    << MSG_SPACE << ERR_MESSAGE << SOCKET_ERRNO;
                log->flush();
            }
        }

        if (bind(sock, (struct sockaddr *) &destAddr, sizeof(destAddr)) < 0) {
            if (log) {
                log->strm(LOG_ERR) << ERR_SOCKET_BIND
                    << MSG_SPACE << ERR_MESSAGE << SOCKET_ERRNO;
                log->flush();
            }
            shutdown(sock, 0);
            close(sock);
            return ERR_CODE_SOCKET_BIND;
        }
        struct sockaddr_storage source_addr{}; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof(source_addr);

        // 307 bytes for IPv4 up to 18, IPv6 up to 10
        unsigned char rBuf[2048];
        while (status != ERR_CODE_STOPPED) {
            ssize_t len = recvfrom(sock, (char*) rxBuf, sizeof(rxBuf) - 1, 0, (struct sockaddr*)&source_addr, & socklen);
            // Error occurred during receiving
            if (len < 0) {
                if (SOCKET_ERRNO == SOCKET_ERROR_TIMEOUT) {    // timeout occurs
                    continue;
                }
                if (log) {
                    log->strm(LOG_ERR) << ERR_SOCKET_READ
                        << MSG_SPACE << ERR_MESSAGE << SOCKET_ERRNO;
                    log->flush();
                }
                continue;
            } else {
                // Data received
                if (log && verbose > 1) {
                    log->strm(LOG_INFO) << MSG_RECEIVED << len << MSG_SPACE << MSG_BYTES << MSG_COLON_N_SPACE << hexString(rxBuf, len);
                    log->flush();
                }
                size_t sz;
                if (len > 0) {
                    sz = identitySerialization->query(rBuf, sizeof(rBuf), rxBuf, len);
                    if (sz == 0) {
                        sz = gatewaySerialization->query(rBuf, sizeof(rBuf), rxBuf, len);
                    }
                } else
                    sz = 0;
                if (sz > 0) {
                    if (sendto(sock, (const char *) rBuf, (int) sz, 0, (struct sockaddr *) &source_addr, sizeof(source_addr)) < 0) {
                        if (log) {
                            log->strm(LOG_ERR) << ERR_SOCKET_WRITE
                                << MSG_SPACE << ERR_MESSAGE << SOCKET_ERRNO;
                            log->flush();
                        }
                    } else {
                        if (log && verbose > 1) {
                            log->strm(LOG_INFO) << MSG_SENT
                                << sz << MSG_SPACE << MSG_BYTES << ": " << hexString(rBuf, sz);
                            log->flush();
                        }
                    }
                } else {
                    if (log && verbose) {
                        log->strm(LOG_ERR) << ERR_INVALID_PACKET << ": " << hexString(rxBuf, len)
                            << " (" << len << MSG_SPACE << MSG_BYTES << ")";
                        log->flush();
                    }
                }
            }
        }
        shutdown(sock, 0);
        close(sock);
    }
    return r;
}

UDPListener::~UDPListener()
{
	stop();
}
