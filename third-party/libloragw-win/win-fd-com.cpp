#include <sstream>
#include "win-fd-com.h"

static int getComNumber(
    const char *winComPortName
) {
    return atoi(winComPortName + 3); // COMx and COMxx
}

WinComPortCollection::WinComPortCollection()
{
    memset(fds, 0, sizeof(fds));
}

int WinComPortCollection::openPort(
    const char *comPortName
)
{
    int fd = getComNumber(comPortName);

    if (fd > MAX_COM_PORT_NUMBER)
        return -1;
    if (!fds[fd]) {
        fds[fd] = new WinComPort(this, comPortName);
        fds[fd]->openHandle();
    }
    return fds[fd]->handle == INVALID_HANDLE_VALUE ? -1 : fd;
}

int WinComPortCollection::closePort(int fd)
{
    if (fd > MAX_COM_PORT_NUMBER)
        return -1;
    if (!fds[fd]) {
        delete fds[fd];
        fds[fd] = nullptr;
    }
    return 0;
}

int WinComPortCollection::readPort(int fd, void* buffer, size_t count)
{
    if (fd > MAX_COM_PORT_NUMBER)
        return -1;
    if (!fds[fd])
        return -1;
    return fds[fd]->readSerial(buffer, count);
}

int WinComPortCollection::writePort(int fd, const void* buffer, size_t count)
{
    if (fd > MAX_COM_PORT_NUMBER)
        return -1;
    if (!fds[fd])
        return -1;
    return fds[fd]->writeSerial(buffer, count);
}

int WinComPortCollection::setSpeed(int fd, int speed)
{
    if (fd > MAX_COM_PORT_NUMBER)
        return -1;
    if (!fds[fd])
        return -1;
    return fds[fd]->setSpeed(speed);
}

int WinComPortCollection::setBlocking(int fd, bool blocking)
{
    if (fd > MAX_COM_PORT_NUMBER)
        return -1;
    if (!fds[fd])
        return -1;
    return fds[fd]->setBlocking(blocking);
}

int WinComPortCollection::setTimeout(int fd, int timeout)
{
    if (fd > MAX_COM_PORT_NUMBER)
        return -1;
    if (!fds[fd])
        return -1;
    return fds[fd]->setTimeout(timeout);
}

WinComPort* WinComPortCollection::getPort(int fd)
{
    if (fd > MAX_COM_PORT_NUMBER)
        return nullptr;
    return fds[fd];
}

WinComPortCollection::~WinComPortCollection()
{

}

WinComPort::WinComPort()
    : collection(nullptr), handle(INVALID_HANDLE_VALUE), fd(-1)
{

}

WinComPort::WinComPort(
    WinComPortCollection *aCollection,
    const char *comPortName
)
    : collection(aCollection)
{
    fd = getComNumber(comPortName);
    auto p = collection->getPort(fd);
    if (!p)
        handle = INVALID_HANDLE_VALUE;
    else
        handle = p->handle;
}

WinComPort::WinComPort(
    const WinComPort& value
)
    : handle(value.handle), fd(value.fd), collection(value.collection)
{

}

WinComPort::~WinComPort()
{
    closeHandle();
}

HANDLE WinComPort::openHandle()
{
    std::stringstream ss;
    ss << "COM" << fd;
    auto flags = GENERIC_READ | GENERIC_WRITE;
    handle = CreateFile(ss.str().c_str(), flags,
        0, nullptr, OPEN_EXISTING, 0, nullptr);
    return handle;
}

int WinComPort::closeHandle(){
    return CloseHandle(handle);
}

int WinComPort::readSerial(void *buffer, size_t count) {
    DWORD readCount;
    int r = ReadFile(handle, buffer, count, &readCount, nullptr);
    if (!r)
        return -1;
    return readCount;
}

int WinComPort::writeSerial(const void *buffer, size_t count) {
    DWORD writeCount;
    int r = WriteFile(handle, buffer, count, &writeCount, nullptr);
    if (!r)
        return -1;
    return writeCount;
}

int WinComPort::setSpeed(int speed) {
    DCB serialParams;
    SecureZeroMemory(&serialParams, sizeof(DCB));
    serialParams.DCBlength = sizeof(DCB);
    GetCommState(handle, &serialParams);
    serialParams.BaudRate = CBR_115200;
    serialParams.fBinary = 1;
    serialParams.ByteSize = 8;
    serialParams.Parity = NOPARITY;
    serialParams.StopBits = ONESTOPBIT;
    return SetCommState(handle, &serialParams) ? 0 : -1;
}

int WinComPort::setBlocking(bool blocking) {
    return setTimeout(blocking ? 0 : 1000);
}

int WinComPort::setTimeout(int timeout)
{
    COMMTIMEOUTS to;
    SecureZeroMemory(&to, sizeof(COMMTIMEOUTS));
    to.ReadTotalTimeoutConstant = timeout;
    return SetCommTimeouts(handle, &to) == 0 ? 0 : -1;
}
