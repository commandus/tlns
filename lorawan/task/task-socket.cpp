#include <cstring>
#include <csignal>

#if defined(_MSC_VER) || defined(__MINGW32__)
#else
#include <sys/ioctl.h>
#define INVALID_SOCKET  (-1)
#endif

#include <sstream>
#include <chrono>

#include "task-socket.h"
#include "lorawan/lorawan-error.h"

TaskSocket::TaskSocket()
    : sock(INVALID_SOCKET), socketAccept(SA_NONE), lastError(CODE_OK), customWrite(false)
{

}

TaskSocket::TaskSocket(
    ENUM_SOCKET_ACCEPT aAccept
)
    : sock(INVALID_SOCKET), socketAccept(aAccept), lastError(CODE_OK), customWrite(false)
{

}

TaskSocket::TaskSocket(
    SOCKET socket,
    ENUM_SOCKET_ACCEPT aAccept
)
    : sock(socket), socketAccept(aAccept), lastError(CODE_OK), customWrite(false)
{

}

void TaskSocket::customWriteSocket(
    const void* data,
    size_t size,
    ProtoGwParser *proto
)
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
        case SA_ACCEPT_REQUIRE:
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

std::string TaskSocket::toJsonString() const
{
    std::stringstream ss;
    ss << "{\"accept\": \"";
    switch (socketAccept) {
        case SA_NONE:
            ss << "none";
            break;
        case SA_ACCEPT_REQUIRE:
            ss << "require";
            break;
        case SA_ACCEPTED:
            ss << "accepted";
            break;
        case SA_TIMER:
            ss << "timer";
            break;
        case SA_EVENTFD:
            ss << "event";
            break;
        default:
            ss << "invalid";
            break;
    }
    ss << "\", \"socket\": " << sock << "}";
    return ss.str();
}

TaskSocketPreNAcceptedSocket::TaskSocketPreNAcceptedSocket()
    : taskSocket(nullptr), acceptedSocket(INVALID_SOCKET)
{

}

TaskSocketPreNAcceptedSocket::TaskSocketPreNAcceptedSocket(
    TaskSocket *aTaskSocket,
    SOCKET aAcceptedSocket
)
    : taskSocket(aTaskSocket), acceptedSocket(aAcceptedSocket)
{

}

GatewayPingTimeNSocket::GatewayPingTimeNSocket()
    : taskSocket(nullptr), tim(std::chrono::system_clock::now())
{

}

GatewayPingTimeNSocket::GatewayPingTimeNSocket(
    TaskSocket *aTaskSocket
)
    : taskSocket(aTaskSocket), tim(std::chrono::system_clock::now())
{

}
