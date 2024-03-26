#include "task-usb-socket.h"

TaskUSBSocket::TaskUSBSocket(
    const std::string &devicePath
)
    : TaskSocket(), path(devicePath)
{

}

SOCKET TaskUSBSocket::openSocket(){
    return -1;
}

void TaskUSBSocket::closeSocket()
{

}

// virtual int onData(const char *buffer, size_t size) = 0;
TaskUSBSocket::~TaskUSBSocket()
{

}
