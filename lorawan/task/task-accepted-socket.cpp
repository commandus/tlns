#include <csignal>

#include "lorawan/task/task-accepted-socket.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#define close(x) closesocket(x)
#endif

TaskAcceptedSocket::TaskAcceptedSocket(
    SOCKET socket
)
    : TaskSocket(socket, SA_ACCEPTED)
{

}

TaskAcceptedSocket::~TaskAcceptedSocket()
{
    closeSocket();
}

SOCKET TaskAcceptedSocket::openSocket()
{
    return sock;
}

void TaskAcceptedSocket::closeSocket()
{
    if (sock >= 0)
        close(sock);
    sock = -1;
}
