#include "task-usb-socket.h"
#include <sys/un.h>
#include <iostream>
#include <fcntl.h>
#include <sys/syslog.h>

#include "lorawan/lorawan-msg.h"

class PosixLibLoragwOpenClose : public LibLoragwOpenClose {
private:
    std::string devicePath;
public:
    explicit PosixLibLoragwOpenClose(const std::string &aDevicePath) : devicePath(aDevicePath) {};

    int openDevice(const char *fileName, int mode) override
    {
        return open(devicePath.c_str(), mode);
    };

    int closeDevice(int fd) override
    {
        return close(fd);
    };
};

static LibLoragwHelper libLoragwHelper;

static void onPushData(
    MessageTaskDispatcher* dispatcher,
    GwPushData &item
)
{

}

/**
 * Open Unix domain socket
 * @param socketFileName Unix domain socket name is file name with owner, group access rights e.g. "/tmp/gw-dev-usb.socket"
 * @param devicePath Gateway device file name e.g. "/dev/ttyACM0"
 */
TaskUSBSocket::TaskUSBSocket(
    MessageTaskDispatcher *aDispatcher,
    const std::string &socketFileName,
    GatewaySettings *aSettings,
    Log *aLog,
    bool enableSend,
    bool enableBeacon,
    int verbosity
)
    : dispatcher(aDispatcher), TaskSocket(),
    socketPath(socketFileName)
{
    listener.config = aSettings;
    listener.setDispatcher(dispatcher);
    listener.flags = (enableSend ? 0 : FLAG_GATEWAY_LISTENER_NO_SEND) | (enableBeacon ? 0 : FLAG_GATEWAY_LISTENER_NO_BEACON);

    if (!aLog)
        verbosity = 0;
    listener.setLogVerbosity(verbosity);
    listener.onLog = aLog;
    listener.setOnPushData(onPushData);
    helperOpenClose = new PosixLibLoragwOpenClose(aSettings->sx130x.boardConf.com_path);
    libLoragwHelper.bind(aLog, helperOpenClose);

    // In case the program exited inadvertently on the last run, remove the socket.
    unlink(socketPath.c_str());

    if (aLog)
        aLog->strm(LOG_INFO) << "Settings " << listener.config->name << "\n";
}

SOCKET TaskUSBSocket::openSocket()
{
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock <= 0)
        return sock;
    struct sockaddr_un sunAddr;
    memset(&sunAddr, 0, sizeof(struct sockaddr_un));

    // Bind socket to socket name
    sunAddr.sun_family = AF_UNIX;
    strncpy(sunAddr.sun_path, socketPath.c_str(), sizeof(sunAddr.sun_path) - 1);
    int r = bind(sock, (const struct sockaddr *) &sunAddr, sizeof(struct sockaddr_un));
    if (r < 0) {
        sock = -1;
        return sock;
    }
    // Prepare for accepting connections. The backlog size is set to 20. So while one request is being processed other requests can be waiting.
    r = listen(sock, 20);
    if (r < 0)
        sock = -1;
    else {
        r = listener.start();
        if (r < 0)
            sock = -1;
    }
    return sock;
}

void TaskUSBSocket::closeSocket()
{
    listener.stop(0);   // default wait 60s
    if (sock > 0) {
        close(sock);
        sock = -1;
    }
    unlink(socketPath.c_str());
}

// virtual int onData(const char *buffer, size_t size) = 0;
TaskUSBSocket::~TaskUSBSocket()
{
    libLoragwHelper.flush();
    closeSocket();

    if (helperOpenClose) {
        delete helperOpenClose;
        helperOpenClose = nullptr;
    }
}

int TaskUSBSocket::listen2() {
    stopped = false;
    while (!stopped) {
        fd_set readHandles;
        FD_ZERO(&readHandles);
        FD_SET(sock, &readHandles);

        struct timeval timeoutInterval;
        timeoutInterval.tv_sec = 1;
        timeoutInterval.tv_usec = 0;

        int rs = select(sock + 1, &readHandles, nullptr, nullptr, &timeoutInterval);
        if (rs == -1) {
            int serrno = errno;
            if (listener.onLog) {
                std::stringstream ss;
                ss << ERR_MESSAGE << ERR_CODE_SELECT << ": " << ERR_SELECT
                   << ", errno " << serrno << ": " << strerror(errno);
                listener.onLog->strm(LOG_WARNING) << ss.str();
            }
            return ERR_CODE_SELECT;
        }
        if (rs == 0) {
            // timeout, nothing to do
            // std::stringstream ss;ss << MSG_TIMEOUT;onInfo(LOG_DEBUG, LOG_UDP_LISTENER, 0, ss.str());
            continue;
        }
        struct timeval receivedTime;
        gettimeofday(&receivedTime, nullptr);
        // By default, there are two sockets: one for IPv4, second for IPv6
        /*
        for (std::vector<UDPSocket>::const_iterator it = sockets.begin(); it != sockets.end(); it++) {
            if (!FD_ISSET(it->sock, &readHandles))
                continue;
            struct sockaddr_in6 gwAddress;
            int bytesReceived = it->recv((void *) buffer.c_str(), buffer.size() - 1,
                                         &gwAddress);    // add extra trailing byte for null-terminated string
            if (bytesReceived <= 0) {
                if (listener.onLog) {
                    std::stringstream ss;
                    ss << ERR_MESSAGE << ERR_CODE_SOCKET_READ << " "
                       << UDPSocket::addrString((const struct sockaddr *) &gwAddress) << ", errno "
                       << errno << ": " << strerror(errno);
                    listener.onLog->strm(LOG_ERR) << LOG_UDP_LISTENER, ERR_CODE_SOCKET_READ, ss.str());
                }
                continue;
            }
            // rapidjson operates with \0 terminated string, just in case add terminator. Extra space is reserved
            buffer[bytesReceived] = '\0';
            if (listener.onLog) {
                std::stringstream ss;
                char *json = SemtechUDPPacket::getSemtechJSONCharPtr(buffer.c_str(), bytesReceived);
                ss << MSG_RECEIVED
                   << UDPSocket::addrString((const struct sockaddr *) &gwAddress)
                   << " (" << bytesReceived
                   << " bytes): " << hexString(buffer.c_str(), bytesReceived);
                if (json)
                    ss << "; " << json;
                listener.onLog(LOG_INFO) << LOG_UDP_LISTENER, 0, ss.str());
            }
            // parseRX packet result code
            int pr = parseBuffer(buffer, bytesReceived, it->sock, receivedTime, gwAddress);
        }
        */
    }
    return CODE_OK;
}
