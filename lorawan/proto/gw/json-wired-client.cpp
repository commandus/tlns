#include "lorawan/proto/gw/json-wired-client.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <WS2tcpip.h>
#define close closesocket
// #define inet_pton InetPtonA
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define INVALID_SOCKET  (-1)
#endif

#include <lorawan/lorawan-error.h>

#define DEF_READ_TIMEOUT_SECONDS    2

JsonWiredClient::JsonWiredClient(
    const DirectClient *aDirectClient,
    uint64_t aGwId,
    const std::string &aNetworkServerAddress,
    uint16_t aNetworkServerPort,
    const DEVADDR &aDeviceAddress
)
    : directClient(aDirectClient), gwId(aGwId), networkServerAddress(aNetworkServerAddress),
    networkServerPort(aNetworkServerPort), deviceAddress(aDeviceAddress),
    sock(INVALID_SOCKET), status(CODE_OK)
{
}

JsonWiredClient::~JsonWiredClient() = default;

int JsonWiredClient::JsonWiredClient::send(
    const std::string &fopts,
    const std::string &payload
)
{
    std::string s;
    write(sock, s.c_str(), s.size());
    return CODE_OK;
}

int JsonWiredClient::JsonWiredClient::run() {
    if (openConnection() != CODE_OK)
        return ERR_CODE_SOCKET_OPEN;
    status = CODE_OK;

    // Set timeout
#ifdef _MSC_VER
    DWORD timeout = DEF_READ_TIMEOUT_SECONDS * 1000;   // ms
#else
    struct timeval timeout { DEF_READ_TIMEOUT_SECONDS, 0 };
#endif
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof timeout);

    while (status != ERR_CODE_STOPPED) {
        struct sockaddr_storage srcAddress{}; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof(srcAddress);
        char rxBuf[1024];
        ssize_t len = recvfrom(sock, (char *) rxBuf, sizeof(rxBuf), 0, (struct sockaddr *)&srcAddress, &socklen);
        if (len < 0) {  // Error occurred during receiving
        } else {
        }
    }
    closeConnection();
    return status;
}

void JsonWiredClient::JsonWiredClient::stop()
{
    status = ERR_CODE_STOPPED;
}

int JsonWiredClient::openConnection() {
    int r = CODE_OK;
    if (isAddrStringIPv6(networkServerAddress.c_str())) {
        sockaddr_in6 a {};
        a.sin6_family = AF_INET6;
        r = inet_pton(AF_INET6, networkServerAddress.c_str(), &a.sin6_addr);
        a.sin6_port = htons(networkServerPort);
        a.sin6_scope_id = 0;
        sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IPV6);
    } else {
        sockaddr_in a {};
        a.sin_family = AF_INET;
        r = inet_pton(AF_INET, networkServerAddress.c_str(), &a.sin_addr);
        a.sin_port = htons(networkServerPort);
        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    }
    if (r == -1)
        status = ERR_CODE_SOCKET_ADDRESS;
    if (sock == INVALID_SOCKET)
        status = ERR_CODE_SOCKET_CREATE;
    return status;
}

void JsonWiredClient::closeConnection() {
    if (sock > 0)
        close(sock);
    sock = INVALID_SOCKET;
}