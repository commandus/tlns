#include "lorawan/task/task-eventfd-control-socket.h"
#include <unistd.h>
#include <sys/eventfd.h>
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-error.h"

TaskEventFDControlSocket::TaskEventFDControlSocket()
    : TaskSocket(SA_EVENTFD)
{
}

SOCKET TaskEventFDControlSocket::openSocket()
{
    sock = eventfd(0, 0);
    if (sock <= 0)
        lastError = ERR_CODE_SOCKET_CREATE;
    return sock;
}

void TaskEventFDControlSocket::closeSocket()
{
    if (sock > 0) {
        close(sock);
        sock = -1;
    }
}

// virtual int onData(const char *buffer, size_t size) = 0;
TaskEventFDControlSocket::~TaskEventFDControlSocket()
{
    closeSocket();
}
