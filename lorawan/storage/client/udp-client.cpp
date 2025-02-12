#include "udp-client.h"

#include <cstring>

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <WS2tcpip.h>

#define close closesocket
#define SOCKET_ERRNO WSAGetLastError()
#define ERR_TIMEOUT WSAETIMEDOUT
// #define inet_pton InetPtonA
#else
#define INVALID_SOCKET  (-1)
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SOCKET_ERRNO errno
#endif

#ifdef ENABLE_DEBUG
#include <iostream>
#include "lorawan-msg.h"
#endif

#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/helper/ip-address.h"
#include "lorawan/storage/serialization/identity-binary-serialization.h"
#include "lorawan/storage/serialization/gateway-binary-serialization.h"

#define DEF_KEEPALIVE_SECS 60
#define DEF_READ_TIMEOUT_SECONDS    2

void UDPClient::stop()
{
    status = ERR_CODE_STOPPED;
}

UDPClient::UDPClient(
        uint32_t ipv4,
        uint16_t aPort,
        ResponseClient *aOnResponse
)
    : sock(0), addr{}, QueryClient(aOnResponse), query(nullptr), status(CODE_OK)
{
    auto *a = (struct sockaddr_in *) &addr;
    a->sin_addr.s_addr = ipv4;
    a->sin_port = htons(aPort);
}

/**
 */
UDPClient::UDPClient(
        const std::string &aHost,
        uint16_t aPort,
        ResponseClient *aOnResponse
)
    : sock(0), addr{}, QueryClient(aOnResponse), query(nullptr), status(CODE_OK)
{
    memset(&addr, 0, sizeof(addr));
    int r;
    if (isAddrStringIPv6(aHost.c_str())) {
        auto *a = (struct sockaddr_in6 *) &addr;
        a->sin6_family = AF_INET6;
        r = inet_pton(AF_INET6, aHost.c_str(), &a->sin6_addr);
        a->sin6_port = htons(aPort);
        a->sin6_scope_id = 0;
    } else {
        auto *a = (struct sockaddr_in *) &addr;
        a->sin_family = AF_INET;
        r = inet_pton(AF_INET, aHost.c_str(), &a->sin_addr);
        a->sin_port = htons(aPort);
    }
    if (r == -1) {
        if (onResponse) {
            onResponse->onError(this, ERR_CODE_SOCKET_ADDRESS, SOCKET_ERRNO);
        }
    }
}

UDPClient::~UDPClient()
{
    stop();
}

ServiceMessage* UDPClient::request(
    ServiceMessage* value
)
{
    ServiceMessage* r = query;
    query = value;
    return r;
}

void UDPClient::start() {
    status = CODE_OK;
    bool ipv6 = isIPv6(&addr);
    int af = ipv6 ? AF_INET6 : AF_INET;
    int proto = ipv6 ? IPPROTO_IPV6 : IPPROTO_IP;

    while (status != ERR_CODE_STOPPED) {
        sock = socket(af, SOCK_DGRAM, proto);
        if (sock == INVALID_SOCKET) {
            status = ERR_CODE_SOCKET_CREATE;
#ifdef ENABLE_DEBUG
            std::cerr << ERR_SOCKET_CREATE << MSG_SPACE << errno << MSG_COLON_N_SPACE << strerror(errno) << std::endl;
#endif
            onResponse->onError(this, ERR_CODE_SOCKET_CREATE, SOCKET_ERRNO);
            break;
        }

        // Set timeout
#ifdef _MSC_VER
        DWORD timeout = DEF_READ_TIMEOUT_SECONDS * 1000;   // ms
#else
        struct timeval timeout { DEF_READ_TIMEOUT_SECONDS, 0 };
#endif
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof timeout);

        unsigned char sendBuffer[110];
        while (status != ERR_CODE_STOPPED) {
            if (!query) {
                status = ERR_CODE_STOPPED;
                break;
            }
            query->ntoh();
            size_t ssz = query->serialize(sendBuffer);
            ssize_t sz = sendto(sock, (const char*) sendBuffer, (int) ssz, 0, &addr, sizeof(addr));
            if (sz < 0) {
                status = ERR_CODE_SOCKET_WRITE;
                onResponse->onError(this, ERR_CODE_SOCKET_WRITE, SOCKET_ERRNO);
                break;
            }
#ifdef ENABLE_DEBUG
            std::cerr << MSG_SENT << sz << MSG_SPACE << MSG_BYTES << MSG_COLON_N_SPACE
            << hexString(sendBuffer, ssz)
            << std::endl;
#endif
            size_t rxSize;
            if (isIdentityTag(sendBuffer, ssz)) {
                rxSize = responseSizeForIdentityRequest(sendBuffer, ssz);
            } else {
                rxSize = responseSizeForGatewayRequest(sendBuffer, ssz);
            }
            struct sockaddr_storage srcAddress{}; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(srcAddress);
            auto *rxBuf = (unsigned char *) malloc(rxSize);

            ssize_t len = recvfrom(sock, (char *) rxBuf, (int) rxSize, 0, (struct sockaddr *)&srcAddress, &socklen);

            if (len < 0) {  // Error occurred during receiving
                status = ERR_CODE_SOCKET_READ;
                onResponse->onError(this, ERR_CODE_SOCKET_READ, SOCKET_ERRNO);
                break;
            } else {
#ifdef ENABLE_DEBUG
                std::cerr << MSG_RECEIVED << len << MSG_SPACE << MSG_BYTES
                << MSG_COLON_N_SPACE << hexString(rxBuf, len)
                << std::endl;
#endif
                if (isIdentityTag(rxBuf, len)) {
                    enum IdentityQueryTag tag = validateIdentityQuery(rxBuf, len);
                    switch (tag) {
                        case QUERY_IDENTITY_EUI:   // request gateway identifier(with address) by network address.
                        case QUERY_IDENTITY_ADDR:   // request gateway address (with identifier) by identifier.
                        {
                            IdentityGetResponse gr(rxBuf, len);
                            gr.ntoh();
                            onResponse->onIdentityGet(this, &gr);
                        }
                            break;
                        case QUERY_IDENTITY_LIST:   // List entries
                        {
                            IdentityListResponse gr(rxBuf, len);
                            gr.response = NTOH4(gr.response);
                            gr.ntoh();
                            onResponse->onIdentityList(this, &gr);
                        }
                            break;
                        default: {
                            IdentityOperationResponse gr(rxBuf, len);
                            gr.ntoh();
                            onResponse->onIdentityOperation(this, &gr);
                        }
                            break;
                    }

                } else {
                    enum GatewayQueryTag tag = validateGatewayQuery(rxBuf, len);
                    switch (tag) {
                        case QUERY_GATEWAY_ADDR:   // request gateway identifier(with address) by network address.
                        case QUERY_GATEWAY_ID:   // request gateway address (with identifier) by identifier.
                        {
                            GatewayGetResponse gr(rxBuf, len);
                            gr.ntoh();
                            onResponse->onGatewayGet(this, &gr);
                        }
                            break;
                        case QUERY_GATEWAY_LIST:   // List entries
                        {
                            GatewayListResponse gr(rxBuf, len);
                            gr.response = NTOH4(gr.response);
                            gr.ntoh();
                            onResponse->onGatewayList(this, &gr);
                        }
                            break;
                        default: {
                            GatewayOperationResponse gr(rxBuf, len);
                            gr.ntoh();
                            onResponse->onGatewayOperation(this, &gr);
                        }
                            break;
                    }
                }
                free(rxBuf);
            }
        }

        if (sock != -1) {
            shutdown(sock, 0);
            close(sock);
        }
    }
}
