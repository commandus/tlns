#include <cstring>
#include <csignal>

#if defined(_MSC_VER) || defined(__MINGW32__)
#else
#include <unistd.h>
#include <sys/ioctl.h>
#define INVALID_SOCKET  (-1)
#endif

#include <fcntl.h>

#include "lorawan/lorawan-error.h"
#include "lorawan/task/task-udp-socket.h"

TaskUDPSocket::TaskUDPSocket(
    in_addr_t aAddr,
    uint16_t aPort
)
    : TaskSocket(SA_NONE), addr(aAddr), port(aPort)
{

}

TaskUDPSocket::~TaskUDPSocket()
{
    closeSocket();
}

SOCKET TaskUDPSocket::openSocket()
{
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        lastError = ERR_CODE_SOCKET_CREATE;
        return INVALID_SOCKET;
    }
    // Allow socket descriptor to be reusable
    int on = 1;
    int rc = setsockopt(sock, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
    if (rc < 0) {
        close(sock);
        sock = INVALID_SOCKET;
        lastError = ERR_CODE_SOCKET_OPEN;
        return INVALID_SOCKET;
    }
    // Set socket to be nonblocking
#if defined(_MSC_VER) || defined(__MINGW32__)
    u_long onw = 1;
    rc = ioctlsocket(sock, FIONBIO, &onw);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    // to make sure use socket specific call
    rc = ioctl(sock, FIONBIO, (char *)&on);
    if (rc < 0) {
        close(sock);
        sock = INVALID_SOCKET;
        lastError = ERR_CODE_SOCKET_OPEN;
        return -1;
    }
#endif
    // Bind the socket
    struct sockaddr_in saddr {};
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
#if defined(_MSC_VER) || defined(__MINGW32__)
    saddr.sin_addr.s_addr = htonl(addr.S_un.S_addr); // inet_pton(AF_INET, addr.c_str(), &(saddr.sin_addr));
#else
    saddr.sin_addr.s_addr = htonl(addr); // inet_pton(AF_INET, addr.c_str(), &(saddr.sin_addr));
#endif
    saddr.sin_port = htons(port);
    rc = bind(sock, (struct sockaddr *) &saddr, sizeof(saddr));
    if (rc < 0) {
        close(sock);
        sock = INVALID_SOCKET;
        lastError = ERR_CODE_SOCKET_BIND;
        return INVALID_SOCKET;
    }
    lastError = CODE_OK;
    return sock;
}

void TaskUDPSocket::closeSocket()
{
    if (sock >= 0)
        close(sock);
    sock = INVALID_SOCKET;
}
