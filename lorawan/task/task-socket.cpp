#include <cstring>
#include <csignal>
#include <sys/ioctl.h>

#include "task-socket.h"
#include "lorawan/lorawan-error.h"

TaskSocket::TaskSocket()
    : sock(-1), lastError(CODE_OK)
{

}

TaskSocket::~TaskSocket()
{
}
