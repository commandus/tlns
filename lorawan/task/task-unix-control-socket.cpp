#include "lorawan/task/task-unix-control-socket.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#else
#include <sys/un.h>
#include <sys/ioctl.h>
#include <unistd.h>
#define INVALID_SOCKET  (-1)
#endif

#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-error.h"

TaskUnixControlSocket::TaskUnixControlSocket(
    const std::string &socketFileName
)
    : TaskSocket(SA_NONE), socketPath(socketFileName)
{
    // In case the program exited inadvertently on the last run, remove the socket.
    unlink(socketPath.c_str());
}

SOCKET TaskUnixControlSocket::openSocket()
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    return INVALID_SOCKET;
#else
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        lastError = ERR_CODE_SOCKET_CREATE;
        return sock;
    }
    struct sockaddr_un sunAddr;
    memset(&sunAddr, 0, sizeof(struct sockaddr_un));
    sunAddr.sun_family = AF_UNIX;
    strncpy(sunAddr.sun_path, socketPath.c_str(), sizeof(sunAddr.sun_path) - 1);

    int on = 1;
    // Allow socket descriptor to be reusable
    int r = setsockopt(sock, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
    if (r < 0) {
        close(sock);
        sock = INVALID_SOCKET;
        lastError = ERR_CODE_SOCKET_OPEN;
        return INVALID_SOCKET;
    }
    // Set socket to be nonblocking
#ifdef _MSC_VER
    u_long onw = 1;
    r = ioctlsocket(sock, FIONBIO, &onw);
#else
    r = ioctl(sock, FIONBIO, (char *)&on);
#endif
    if (r < 0) {
        close(sock);
        sock = INVALID_SOCKET;
        lastError = ERR_CODE_SOCKET_OPEN;
        return INVALID_SOCKET;
    }
    /*

    r = bind(sock, (const struct sockaddr *) &sunAddr, sizeof(struct sockaddr_un));
    if (r < 0) {
        sock = INVALID_SOCKET;
        lastError = ERR_CODE_SOCKET_BIND;
        return sock;
    }

    // Prepare for accepting connections. The backlog size is set to 20. So while one request is being processed other requests can be waiting.
    r = listen(sock, 20);
    if (r < 0) {
        sock = INVALID_SOCKET;
        return sock;
    }
*/
    /* 2025-02-26 connect replaced by listen()! */

    r = connect(sock, (const struct sockaddr *) &sunAddr, sizeof(struct sockaddr_un));
    if (r < 0) {
        sock = INVALID_SOCKET;
        lastError = ERR_CODE_SOCKET_CONNECT;
        return sock;
    }
    return sock;
#endif
}

void TaskUnixControlSocket::closeSocket()
{
    if (sock != INVALID_SOCKET) {
        close(sock);
        sock = INVALID_SOCKET;
    }
    unlink(socketPath.c_str());
}

// virtual int onData(const char *buffer, size_t size) = 0;
TaskUnixControlSocket::~TaskUnixControlSocket()
{
    closeSocket();
}
