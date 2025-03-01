#include <cstring>

#if defined(_MSC_VER) || defined(__MINGW32__)
#define close closesocket
#define write(sock, b, sz) ::send(sock, b, sz, 0)
#else
#include <sys/ioctl.h>
#include <unistd.h>
#define INVALID_SOCKET  (-1)
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
        // if address is invalid, assign IPv4 loop-back interface and any random port number
        si->sin_family = AF_INET;
//      si->sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
        si->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        si->sin_port = 0;   // TCP/IP stack assign random port number
    }
    memmove(&addr, &si->sin_addr, sizeof(in_addr_t));
    port = htons(si->sin_port);
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
    u_long onw = 1;
    rc = ioctlsocket(sock, FIONBIO, &onw);
#else
    rc = ioctl(sock, FIONBIO, (char *) &on);
#endif
    if (rc < 0) {
        close(sock);
        sock = -1;
        lastError = ERR_CODE_SOCKET_OPEN;
        return -1;
    }
    // Connect
    struct sockaddr_in saddr {};
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
#if defined(_MSC_VER) || defined(__MINGW32__)
    saddr.sin_addr.s_addr = addr.S_un.S_addr;
#else
    saddr.sin_addr.s_addr = addr;
#endif
    saddr.sin_port = htons(port);

    // std::cout << "Connect control UDP socket " << sockaddr2string((sockaddr*) &saddr) << std::endl;

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
