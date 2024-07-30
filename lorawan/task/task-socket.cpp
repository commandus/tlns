#include <cstring>
#include <csignal>
#include <sys/ioctl.h>
#include <sstream>

#include "task-socket.h"
#include "lorawan/lorawan-error.h"

TaskSocket::TaskSocket()
    : sock(-1), socketAccept(SA_NONE), lastError(CODE_OK)
{

}

TaskSocket::TaskSocket(
    ENUM_SOCKET_ACCEPT aAccept
)
    : sock(-1), socketAccept(aAccept), lastError(CODE_OK)
{

}

TaskSocket::TaskSocket(
    SOCKET socket,
    ENUM_SOCKET_ACCEPT aAccept
)
    : sock(socket), socketAccept(aAccept), lastError(CODE_OK)
{

}

TaskSocket::~TaskSocket()
{
}

std::string TaskSocket::toString() const
{
    std::stringstream ss;
    switch (socketAccept) {
        case SA_NONE:
            ss << "socket do not require accept ";
            break;
        case SA_REQUIRE:
            ss << "socket require accept ";
            break;
        case SA_ACCEPTED:
            ss << "accepted socket ";
            break;
        case SA_TIMER:
            ss << "timer socket ";
            break;
        case SA_EVENTFD:
            ss << "event reserved socket ";
            break;
        default:
            ss << "invalid socket ";
            break;
    }
    ss << sock;
    return ss.str();
}
