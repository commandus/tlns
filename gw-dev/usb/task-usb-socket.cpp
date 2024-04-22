#include "task-usb-socket.h"
#include <sys/un.h>
#include <iostream>
#include <fcntl.h>
#include <sys/syslog.h>
#include <sys/ioctl.h>

#include "lorawan/lorawan-msg.h"
#include "lorawan/lorawan-string.h"

class PosixLibLoragwOpenClose : public LibLoragwOpenClose {
private:
    std::string devicePath;
public:
    explicit PosixLibLoragwOpenClose(
        const std::string &aDevicePath
    ) : devicePath(aDevicePath)
    {

    }
    int openDevice(
        const char *fileName,
        int mode
    ) override
    {
        return open(devicePath.c_str(), mode);
    }

    int closeDevice(
        int fd
    ) override
    {
        return close(fd);
    }
};

static LibLoragwHelper libLoragwHelper;

static void onPushData(
    MessageTaskDispatcher* dispatcher,
    SEMTECH_PROTOCOL_METADATA_RX metadata,
    void *radioPacket,
    size_t size
)
{
    GwPushData pd;
    setLORAWAN_MESSAGE_STORAGE(pd.rxData, radioPacket, size);
    pd.rxMetadata = metadata;
    dispatcher->pushData(pd);
}

/**
 * Open Unix domain socket
 * @param socketFileName Unix domain socket name is file name with owner, group access rights e.g. "/tmp/gw-dev-usb.socket"
 * @param devicePath Gateway device file name e.g. "/dev/ttyACM0"
 */
TaskUsbGatewayUnixSocket::TaskUsbGatewayUnixSocket(
    MessageTaskDispatcher *aDispatcher,
    const std::string &socketFileName,
    GatewaySettings *aSettings,
    Log *aLog,
    bool enableSend,
    bool enableBeacon,
    int verbosity
)
    : dispatcher(aDispatcher), TaskSocket(SA_REQUIRE),
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
    /*
    listener.setOnPullResp();
    listener.setOnTxpkAck();
    listener.setOnSpectralScan();
     */
    helperOpenClose = new PosixLibLoragwOpenClose(aSettings->sx130x.boardConf.com_path);
    libLoragwHelper.bind(aLog, helperOpenClose);

    // In case the program exited inadvertently on the last run, remove the socket.
    unlink(socketPath.c_str());

    if (aLog)
        aLog->strm(LOG_INFO) << "Settings " << listener.config->name << "\n";
}

SOCKET TaskUsbGatewayUnixSocket::openSocket()
{
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock <= 0)
        return sock;
    struct sockaddr_un sunAddr;
    memset(&sunAddr, 0, sizeof(struct sockaddr_un));

    // Allow socket descriptor to be reusable
    int on = 1;
    int rc = setsockopt(sock, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
    if (rc < 0) {
        close(sock);
        sock = -1;
        lastError = ERR_CODE_SOCKET_OPEN;
        return -1;
    }
    // Set socket to be nonblocking
#ifdef _MSC_VER
#else
    rc = ioctl(sock, FIONBIO, (char *)&on);
    if (rc < 0) {
        close(sock);
        sock = -1;
        lastError = ERR_CODE_SOCKET_OPEN;
        return -1;
    }
#endif

    // Bind socket to socket name
    sunAddr.sun_family = AF_UNIX;
    strncpy(sunAddr.sun_path, socketPath.c_str(), sizeof(sunAddr.sun_path) - 1);
    int r = bind(sock, (const struct sockaddr *) &sunAddr, sizeof(struct sockaddr_un));
    if (r < 0) {
        sock = -1;
        lastError = ERR_CODE_SOCKET_BIND;
        return sock;
    }
    // Prepare for accepting connections. The backlog size is set to 20. So while one request is being processed other requests can be waiting.
    r = listen(sock, 20);
    if (r < 0) {
        sock = -1;
        return sock;
    }

    r = listener.start();
    if (r < 0) {
        sock = -1;
        lastError = ERR_CODE_SOCKET_LISTEN;
    }
    return sock;
}

void TaskUsbGatewayUnixSocket::closeSocket()
{
    listener.stop(0);   // default wait 60s
    if (sock > 0) {
        close(sock);
        sock = -1;
    }
    unlink(socketPath.c_str());
}

// virtual int onData(const char *buffer, size_t size) = 0;
TaskUsbGatewayUnixSocket::~TaskUsbGatewayUnixSocket()
{
    libLoragwHelper.flush();
    closeSocket();

    if (helperOpenClose) {
        delete helperOpenClose;
        helperOpenClose = nullptr;
    }
}
