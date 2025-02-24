#include "task-usb-socket.h"
#include <iostream>

#if defined(_MSC_VER) || defined(__MINGW32__)
#else
#include <sys/un.h>
#include <sys/syslog.h>
#include <sys/ioctl.h>
#endif

#include "lorawan/lorawan-msg.h"
#include "lorawan/lorawan-string.h"

static void onPushData(
    MessageTaskDispatcher* dispatcher,
    const TaskSocket *taskSocket,
    const sockaddr &sockAddr,
    SEMTECH_PROTOCOL_METADATA_RX metadata,
    void *radioPacket,
    size_t size
)
{
    GwPushData pd;
    setLORAWAN_MESSAGE_STORAGE(pd.rxData, radioPacket, size);
    pd.rxMetadata = metadata;
    ProtoGwParser *p = dispatcher->parsers.size() ? dispatcher->parsers[0] : nullptr;
    dispatcher->pushData(taskSocket, sockAddr, pd, std::chrono::system_clock::now(), p);
}

static bool onReceiveRawData(
    MessageTaskDispatcher* dispatcher,
    const char *buffer,
    size_t bufferSize,
    TASK_TIME receivedTime
)
{
    if (dispatcher->onReceiveRawData)
        dispatcher->onReceiveRawData(dispatcher, buffer, bufferSize, receivedTime);
    return true;
}

/**
 * Open Unix domain socket
 * @param socketFileNameOrAddress Unix domain socket name is file name with owner, group access rights e.g. "/tmp/gw-dev-usb.socket"
 * @param devicePath Gateway device file name e.g. "/dev/ttyACM0"
 */
TaskUsbGatewaySocket::TaskUsbGatewaySocket(
    MessageTaskDispatcher *aDispatcher,
    const std::string &socketFileNameOrAddress,
    GatewaySettings *aSettings,
    Log *aLog,
    bool enableSend,
    bool enableBeacon,
    int verbosity
)
    : TaskSocket(SA_REQUIRE), dispatcher(aDispatcher), socketNameOrAddress(socketFileNameOrAddress)
{
    listener.socket = this;
    if (!aLog)
        verbosity = 0;
    listener.init(aSettings, aLog);
    listener.setDispatcher(dispatcher);
    listener.setProtocolParser(parser);
    listener.flags = (enableSend ? 0 : FLAG_GATEWAY_LISTENER_NO_SEND) | (enableBeacon ? 0 : FLAG_GATEWAY_LISTENER_NO_BEACON);

    listener.setLogVerbosity(verbosity);
    listener.setOnPushData(onPushData);
    listener.setOnReceiveRawData(onReceiveRawData);
    /*
    listener.setOnPullResp();
    listener.setOnTxpkAck();
    listener.setOnSpectralScan();
     */
#ifdef _MSC_VER
#else
    // In case the program exited inadvertently on the last run, remove the socket.
    unlink(socketNameOrAddress.c_str());
#endif
    if (aLog)
        aLog->log(LOG_INFO, "Settings " + listener.config->name);
}

SOCKET TaskUsbGatewaySocket::openSocket()
{
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock <= 0)
        return sock;
#ifdef _MSC_VER
    struct sockaddr_in sunAddr;
    memset(&sunAddr, 0, sizeof(struct sockaddr_in));
    sunAddr.sin_family = AF_INET;
#else
    struct sockaddr_un sunAddr;
    memset(&sunAddr, 0, sizeof(struct sockaddr_un));
    sunAddr.sun_family = AF_UNIX;
#endif
    int on = 1;
    // Allow socket descriptor to be reusable
    int rc = setsockopt(sock, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
    if (rc < 0) {
        close(sock);
        sock = -1;
        lastError = ERR_CODE_SOCKET_OPEN;
        return -1;
    }
    // Set socket to be nonblocking
#ifdef _MSC_VER
    u_long onw = 1;
    rc = ioctlsocket(sock, FIONBIO, &onw);
#else
    rc = ioctl(sock, FIONBIO, (char *)&on);
#endif
    if (rc < 0) {
        close(sock);
        sock = -1;
        lastError = ERR_CODE_SOCKET_OPEN;
        return -1;
    }

    // Bind socket to socket name
#ifdef _MSC_VER
    string2sockaddr((struct sockaddr*) &sunAddr, socketNameOrAddress);
    int r = bind(sock, (const struct sockaddr *) &sunAddr, sizeof(struct sockaddr_in));
#else
    strncpy(sunAddr.sun_path, socketNameOrAddress.c_str(), sizeof(sunAddr.sun_path) - 1);
    int r = bind(sock, (const struct sockaddr *) &sunAddr, sizeof(struct sockaddr_un));
#endif
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

void TaskUsbGatewaySocket::closeSocket()
{
    listener.stop(0);   // default wait 60s
    if (sock > 0) {
        close(sock);
        sock = -1;
    }
#ifdef _MSC_VER
#else
    unlink(socketNameOrAddress.c_str());
#endif
}

// virtual int onData(const char *buffer, size_t size) = 0;
TaskUsbGatewaySocket::~TaskUsbGatewaySocket()
{
    closeSocket();
}
