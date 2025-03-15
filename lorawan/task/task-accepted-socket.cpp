#include <csignal>

#include "lorawan/task/task-accepted-socket.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#define close(x) closesocket(x)
#else
#include <unistd.h>
#endif

TaskAcceptedSocket::TaskAcceptedSocket(
    TaskSocketPreNAcceptedSocket &taskSocketPreNAcceptedSocket
)
    : TaskSocket(taskSocketPreNAcceptedSocket.acceptedSocket, SA_ACCEPTED)
{
    if (taskSocketPreNAcceptedSocket.taskSocket) {
        // copy custom write from the originator
        customWrite = taskSocketPreNAcceptedSocket.taskSocket->customWrite;
        originator = taskSocketPreNAcceptedSocket.taskSocket;
    } else {
        originator = nullptr;
    }
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

void TaskAcceptedSocket::customWriteSocket(
    const void* data,
    size_t size,
    ProtoGwParser *proto
) {
    if (originator)
        originator->customWriteSocket(data, size, proto);
}
