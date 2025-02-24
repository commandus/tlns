#include <cstring>
#include <csignal>

#if defined(_MSC_VER) || defined(__MINGW32__)
#define close closesocket
#define write(sock, b, sz) ::send(sock, b, sz, 0)
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include "lorawan/lorawan-error.h"
#include "lorawan/task/task-udp-control-socket.h"
#include "lorawan/helper/ip-address.h"

TaskUDPControlSocket::TaskUDPControlSocket(
    in_addr_t aAddr,
    uint16_t aPort
)
    : TaskSocket(SA_NONE), addr(aAddr), port(aPort)
{

}

TaskUDPControlSocket::TaskUDPControlSocket(
    const std::string &addrNPort
)
{
    sockaddr sa;
    sockaddr_in* si = (sockaddr_in*) &sa;
    if (!string2sockaddr(&sa, addrNPort)) {
        // if address is invalid, assign loop-back interface and any random port number
        si->sin_family = AF_INET;
        si->sin_addr.S_un.S_addr = INADDR_LOOPBACK;
        si->sin_port = 0;   // TCP/IP stack assign random port number
    }
    memmove(&addr, &si->sin_addr, sizeof(in_addr_t));
}

TaskUDPControlSocket::~TaskUDPControlSocket()
{
    closeSocket();
}

SOCKET TaskUDPControlSocket::openSocket()
{
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
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
#if defined(_MSC_VER) || defined(__MINGW32__)
#else
    rc = ioctl(sock, FIONBIO, (char *)&on);
    if (rc < 0) {
        close(sock);
        sock = -1;
        lastError = ERR_CODE_SOCKET_OPEN;
        return -1;
    }
#endif
    // Connect
    struct sockaddr_in saddr {};
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
#if defined(_MSC_VER) || defined(__MINGW32__)
    saddr.sin_addr.s_addr = htonl(addr.S_un.S_addr); // inet_pton(AF_INET, addr.c_str(), &(saddr.sin_addr));
#else
    saddr.sin_addr.s_addr = htonl(addr); // inet_pton(AF_INET, addr.c_str(), &(saddr.sin_addr));
#endif
    saddr.sin_port = htons(port);
    rc = connect(sock, (struct sockaddr *) &saddr, sizeof(saddr));
    if (rc < 0) {
        close(sock);
        sock = -1;
        lastError = ERR_CODE_SOCKET_CONNECT;
        return -1;
    }
    lastError = CODE_OK;
    return sock;
}

void TaskUDPControlSocket::closeSocket()
{
    if (sock != INVALID_SOCKET)
        close(sock);
    sock = INVALID_SOCKET;
}
