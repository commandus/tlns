#include "lorawan/task/task-eventfd-control-socket.h"
#include <unistd.h>
#include <sys/eventfd.h>
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-error.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#else
#define INVALID_SOCKET  (-1)
#endif

TaskEventFDControlSocket::TaskEventFDControlSocket()
    : TaskSocket(SA_EVENTFD)
{
}

SOCKET TaskEventFDControlSocket::openSocket()
{
    sock = eventfd(0, 0);
    if (sock == INVALID_SOCKET)
        lastError = ERR_CODE_SOCKET_CREATE;
    return sock;
}

void TaskEventFDControlSocket::closeSocket()
{
    if (sock != INVALID_SOCKET) {
        close(sock);
        sock = INVALID_SOCKET;
    }
}

// virtual int onData(const char *buffer, size_t size) = 0;
TaskEventFDControlSocket::~TaskEventFDControlSocket()
{
    closeSocket();
}
