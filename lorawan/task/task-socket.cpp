#include <cstring>
#include <csignal>
#include <sys/ioctl.h>

#include "task-socket.h"
#include "lorawan/lorawan-error.h"

TaskSocket::TaskSocket()
    : sock(-1), accept(SA_NONE), lastError(CODE_OK)
{

}

TaskSocket::TaskSocket(
    ENUM_SOCKET_ACCEPT aAccept
)
    : sock(-1), accept(aAccept), lastError(CODE_OK)
{

}

TaskSocket::TaskSocket(
    SOCKET socket,
    ENUM_SOCKET_ACCEPT aAccept
)
    : sock(socket), accept(aAccept), lastError(CODE_OK)
{

}

TaskSocket::~TaskSocket()
{
}
