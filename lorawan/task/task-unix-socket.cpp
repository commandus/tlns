#include <csignal>
#include <sys/ioctl.h>
#include <sys/un.h>

#include "lorawan/task/task-unix-socket.h"
#include "lorawan/lorawan-error.h"

TaskUnixSocket::TaskUnixSocket(
    const std::string &socketFileName
)
    : TaskSocket(SA_REQUIRE), socketPath(socketFileName)
{
    unlink(socketPath.c_str());
}

TaskUnixSocket::~TaskUnixSocket()
{
    closeSocket();
}

SOCKET TaskUnixSocket::openSocket()
{
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock <= 0)
        return sock;
    struct sockaddr_un sunAddr;
    memset(&sunAddr, 0, sizeof(struct sockaddr_un));

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

    // Bind socket to socket name
    sunAddr.sun_family = AF_UNIX;
    strncpy(sunAddr.sun_path, socketPath.c_str(), sizeof(sunAddr.sun_path) - 1);
    int r = bind(sock, (const struct sockaddr *) &sunAddr, sizeof(struct sockaddr_un));
    if (r < 0) {
        sock = -1;
        lastError = ERR_CODE_SOCKET_BIND;
        return sock;
    }
    // Prepare for accepting connections. The backlog size is set to 20. So while one request is being processed other requests can be waiting.
    r = listen(sock, 20);
    if (r < 0) {
        sock = -1;
        return sock;
    }
    return sock;
}

void TaskUnixSocket::closeSocket()
{
    if (sock >= 0)
        close(sock);
    sock = -1;
    unlink(socketPath.c_str());
}
