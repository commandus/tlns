#include <iostream>
#include "task-usb-socket.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#define TASK_SOCKET_ACCEPT SA_NONE
#else
#include <sys/un.h>
#include <sys/syslog.h>
#include <sys/ioctl.h>
#define INVALID_SOCKET  (-1)
#define TASK_SOCKET_ACCEPT SA_ACCEPT_REQUIRE
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
    : TaskSocket(TASK_SOCKET_ACCEPT), dispatcher(aDispatcher), socketNameOrAddress(socketFileNameOrAddress)
{
    // enable direct write to
    customWrite = true;

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
#ifdef _MSC_VER
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
        return sock;
    struct sockaddr_in sunAddr;
    memset(&sunAddr, 0, sizeof(struct sockaddr_in));
    sunAddr.sin_family = AF_INET;
#else
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
        return sock;
    struct sockaddr_un sunAddr;
    memset(&sunAddr, 0, sizeof(struct sockaddr_un));
    sunAddr.sun_family = AF_UNIX;
#endif
    int on = 1;
    // Allow socket descriptor to be reusable
    int r = setsockopt(sock, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
    if (r < 0) {
        close(sock);
        sock = INVALID_SOCKET;
        lastError = ERR_CODE_SOCKET_OPEN;
        return INVALID_SOCKET;
    }
    // Set socket to be nonblocking
#ifdef _MSC_VER
    u_long onw = 1;
    r = ioctlsocket(sock, FIONBIO, &onw);
#else
    r = ioctl(sock, FIONBIO, (char *)&on);
#endif
    if (r < 0) {
        close(sock);
        sock = INVALID_SOCKET;
        lastError = ERR_CODE_SOCKET_OPEN;
        return INVALID_SOCKET;
    }
    // Bind socket to socket name
#ifdef _MSC_VER
    bool addressSpecified = string2sockaddr((struct sockaddr*) &sunAddr, socketNameOrAddress);
    if (!addressSpecified) {
        // if address is not assigned or invalid, assign loop-back interface and any random port number
        sunAddr.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
        sunAddr.sin_port = 0;   // TCP/IP stack assign random port number
    }

    r = bind(sock, (const struct sockaddr *) &sunAddr, sizeof(struct sockaddr_in));

    // std::cout << "USB Listen UDP socket " << sockaddr2string((sockaddr*) &sunAddr) << std::endl;

    if (!addressSpecified) {
        int nameLen = sizeof(struct sockaddr_in);
        getsockname(sock, (sockaddr *) &sunAddr, &nameLen);
        uint16_t nPort = htons(sunAddr.sin_port); // random assigned UDP port (in network byte order).
        socketNameOrAddress = "127.0.0.1:" + std::to_string(nPort);
    }
#else
    strncpy(sunAddr.sun_path, socketNameOrAddress.c_str(), sizeof(sunAddr.sun_path) - 1);
    r = bind(sock, (const struct sockaddr *) &sunAddr, sizeof(struct sockaddr_un));
#endif
    if (r < 0) {
        sock = INVALID_SOCKET;
        lastError = ERR_CODE_SOCKET_BIND;
        return sock;
    }
    r = listener.start();
    if (r < 0) {
        sock = INVALID_SOCKET;
        lastError = ERR_CODE_SOCKET_LISTEN;
    }
    // Prepare for accepting connections. The backlog size is set to 20. So while one request is being processed other requests can be waiting.
#ifndef _MSC_VER
    r = listen(sock, 20);
    if (r < 0) {
        sock = INVALID_SOCKET;
        return sock;
    }
#endif
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

void TaskUsbGatewaySocket::customWriteSocket(
    const NetworkIdentity *networkIdentity,
    const void* data,
    size_t size,
    ProtoGwParser *proto
)
{
    std::cerr << "TaskUsbGatewaySocket::customWriteSocket " << hexString(data, size) << std::endl;
    std::cerr << "TaskUsbGatewaySocket::customWriteSocket " << std::string((const char *) data + 12, size - 12) << std::endl;
    ParseResult pr;
    TASK_TIME receivedTime = std::chrono::system_clock::now();
    proto->parse(pr, (const char *) data, size, receivedTime);

    TxPacket tx;
    tx.pkt.freq_hz = pr.gwPullData.txMetadata.freq_hz;          // uint32_t center frequency of TX
    tx.pkt.tx_mode = pr.gwPullData.txMetadata.tx_mode;          // uint8_t select on what event/time the TX is triggered
    tx.pkt.count_us = pr.gwPullData.txMetadata.count_us;        // uint32_t timestamp or delay in microseconds for TX trigger
    tx.pkt.rf_chain = pr.gwPullData.txMetadata.rf_chain;        // uint8_t through which RF chain will the packet be sent
    tx.pkt.rf_power = pr.gwPullData.txMetadata.rf_power;        // int8_t TX power, in dBm
    tx.pkt.modulation = pr.gwPullData.txMetadata.modulation;    // uint8_t modulation to use for the packet
    tx.pkt.freq_offset = 0;                                     // int8_t frequency offset from Radio Tx frequency (CW mode)
    tx.pkt.bandwidth = pr.gwPullData.txMetadata.bandwidth;      // uint8_t modulation bandwidth (LoRa only)
    tx.pkt.datarate = pr.gwPullData.txMetadata.datarate;        // uint32_t TX datarate (baudrate for FSK, SF for LoRa)
    tx.pkt.coderate = pr.gwPullData.txMetadata.coderate;        // uint8_t error-correcting code of the packet (LoRa only)
    tx.pkt.invert_pol = pr.gwPullData.txMetadata.invert_pol;    // bool invert signal polarity, for orthogonal downlinks (LoRa only)
    tx.pkt.f_dev = pr.gwPullData.txMetadata.f_dev;              // uint8_t frequency deviation, in kHz (FSK only)
    tx.pkt.preamble = pr.gwPullData.txMetadata.preamble;        // uint16_t set the preamble length, 0 for default
    tx.pkt.no_crc = pr.gwPullData.txMetadata.no_crc;            // bool if true, do not send a CRC in the packet
    tx.pkt.no_header = pr.gwPullData.txMetadata.no_header;      // bool if true, enable implicit header mode (LoRa), fixed length (FSK)
    tx.pkt.size = pr.gwPullData.txMetadata.size;                // uint16_t payload size in bytes
    std::cerr << "RAK2287 enqueueTxPacket size "
        << (int) tx.pkt.size << " payload size " << (int) pr.gwPullData.txData.payloadSize << " header size "
        << (int) (tx.pkt.size - pr.gwPullData.txData.payloadSize)
        << "\nmetadata: " << SEMTECH_PROTOCOL_METADATA_TX2string(pr.gwPullData.txMetadata)
        << "\ntxData: " << pr.gwPullData.txData.toString()
        << std::endl;
    size_t sz = pr.gwPullData.txData.toArray(tx.pkt.payload, sizeof(tx.pkt.payload), networkIdentity);      // uint8_t buffer containing the payload
    std::cerr << "payload: " << hexString(tx.pkt.payload, sz) << " size " << sz << std::endl;
    listener.enqueueTxPacket(tx);
}
