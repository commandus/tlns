#include <cstring>
#include <csignal>
#include <sys/ioctl.h>

#include "lorawan/lorawan-error.h"
#include "lorawan/task/task-udp-socket.h"

TaskUDPSocket::TaskUDPSocket(
    in_addr_t aAddr,
    uint16_t aPort
)
    : TaskSocket(), addr(aAddr), port(aPort)
{

}

TaskUDPSocket::~TaskUDPSocket()
{
    closeSocket();
}

SOCKET TaskUDPSocket::openSocket()
{
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        lastError = ERR_CODE_SOCKET_CREATE;
        return -1;
    }
    // Allow socket descriptor to be reusable
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
    // Prepare for accepting connections. The backlog size is set to 20. So while one request is being processed other requests can be waiting.
    /* UDP do not require listen()
    rc = listen(sock, 20);
    if (rc < 0) {
        sock = -1;
        return sock;
    }
    */
    lastError = CODE_OK;
    return sock;
}

void TaskUDPSocket::closeSocket()
{
    if (sock >= 0)
        close(sock);
    sock = -1;
}
