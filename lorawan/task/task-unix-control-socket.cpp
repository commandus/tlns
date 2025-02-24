#include "lorawan/task/task-unix-control-socket.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#else
#include <sys/un.h>
#include <unistd.h>
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
    if (sock <= 0) {
        lastError = ERR_CODE_SOCKET_CREATE;
        return sock;
    }
    struct sockaddr_un sunAddr;
    memset(&sunAddr, 0, sizeof(struct sockaddr_un));
    sunAddr.sun_family = AF_UNIX;
    strncpy(sunAddr.sun_path, socketPath.c_str(), sizeof(sunAddr.sun_path) - 1);
    int r = connect(sock, (const struct sockaddr *) &sunAddr, sizeof(struct sockaddr_un));
    if (r < 0) {
        sock = -1;
        lastError = ERR_CODE_SOCKET_CONNECT;
        return sock;
    }
    return sock;
#endif
}

void TaskUnixControlSocket::closeSocket()
{
    if (sock > 0) {
        close(sock);
        sock = -1;
    }
    unlink(socketPath.c_str());
}

// virtual int onData(const char *buffer, size_t size) = 0;
TaskUnixControlSocket::~TaskUnixControlSocket()
{
    closeSocket();
}
