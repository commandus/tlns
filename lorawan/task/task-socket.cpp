#include <cstring>
#include <csignal>
#include <sys/ioctl.h>

#include "task-socket.h"
#include "lorawan/lorawan-error.h"

TaskSocket::TaskSocket(
    in_addr_t aAddr,
    uint16_t aPort
)
    : sock(-1), addr(aAddr), port(aPort), lastError(CODE_OK)
{

}

TaskSocket::~TaskSocket()
{
    closeSocket();
}

SOCKET TaskSocket::openSocket()
{
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        lastError = ERR_CODE_SOCKET_CREATE;
        return -1;
    }
    // Allow socket descriptor to be reuseable
    int on = 1;
    int rc = setsockopt(sock, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
    if (rc < 0) {
        close(sock);
        sock = -1;
        lastError = ERR_CODE_SOCKET_OPEN;
        return -1;
    }
    // Set socket to be nonblocking
#ifdef _MSC_VER
#else
    rc = ioctl(sock, FIONBIO, (char *)&on);
    if (rc < 0) {
        close(sock);
        sock = -1;
        lastError = ERR_CODE_SOCKET_OPEN;
        return -1;
    }
#endif
    // Bind the socket
    struct sockaddr_in saddr {};
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
#if defined(_MSC_VER)
    saddr.sin_addr.s_addr = htonl(addr.S_un.S_addr); // inet_pton(AF_INET, addr.c_str(), &(saddr.sin_addr));
#else
    saddr.sin_addr.s_addr = htonl(addr); // inet_pton(AF_INET, addr.c_str(), &(saddr.sin_addr));
#endif
    saddr.sin_port = htons(port);
    rc = bind(sock, (struct sockaddr *) &saddr, sizeof(saddr));
    if (rc < 0) {
        close(sock);
        sock = -1;
        lastError = ERR_CODE_SOCKET_BIND;
        return -1;
    }
    lastError = CODE_OK;
    return sock;
}

void TaskSocket::closeSocket()
{
    if (sock >= 0)
        close(sock);
    sock = -1;
}
