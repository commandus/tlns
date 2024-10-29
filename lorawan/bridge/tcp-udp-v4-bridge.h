#ifndef TLNS_TCP_UDP_V4_BRIDGE_H
#define TLNS_TCP_UDP_V4_BRIDGE_H

/**
 * Example how to implement TCP/UDP service
 * Clients (up to 1010) can connect to any interface at port 4250 (by default).
 * init()
 *
 * Files:
 *      Unix socket "/tmp/tcp-udp-v4-bridge.socket" used internally to route payload from the gateway(s) to clients.
 * Tou can change Unix socket file name in second parameter of init()
 *
 * Test with netcat:
 * TCP
 *      nc 127.0.0.1 4250 -w 3600
 * UDP
 *      nc -u 127.0.0.1 4250 -w 3600
 *          send smth
 *
 * Note about TCP clients:
 *
 * Service can serve up to 1010 clients (1024 minus listen and other sockets). If there are no resources,
 * new TCP connection immediately closed by the service.
 *
 * Note about UDP clients:
 *
 * After UDP client after socket initialization must write any message to the UDP port.
 * Message content ignored but it is important because UDP server must receive any packet
 * to remember client's address and port number.
 * Then after gateway(s) send payload, service send payload to all stored addresses.
 * If there are more than 1010 clients, service forgot "oldest" client to free up resources.
 * "Oldest" client is a UDP connected client who send any bytes to the service ("ping").
 * Therefore client can periodically send "ping" packet to the service.
 * If all sockets occupied by TCP clients, no any UDP clients can receive payload from the service.
 *
 */
#include "lorawan/bridge/app-bridge.h"

#include <string>
#include <cinttypes>
#include <thread>

#if defined(_MSC_VER) || defined(__MINGW32__)
#else
typedef int SOCKET;
#endif

class TcpUdpV4Bridge : public AppBridge {
private:
    SOCKET tcpListenSocket;
    SOCKET udpSocket;
    SOCKET onPayloadListenSocket;
    SOCKET onPayloadAcceptedSocket;
    SOCKET onPayloadClientSocket;
    std::string addrAndPort;
    std::string onPayloadSocketPath;
    std::thread *thread;
    bool running;
    bool stopped;
    int openOnPayloadSocket();
    void closeOnPayloadSocket();
    int openSockets();
    void closeSockets();

    int start();
    void stop();
    void run();
public:
    TcpUdpV4Bridge();
    virtual ~TcpUdpV4Bridge() = default;
    void onPayload(
        const void *dispatcher,
        const MessageQueueItem *messageItem,
        bool decoded,
        bool micMatched
    ) override;

    /**
     *
     * @param option    <address>:<port number>. "*:7890 or :7890"- any interface, port 7890, "*" or ""- any interface, port 4250
     * @param option2   unix socket file name. Default "/tmp/tcp-udp-v4-bridge.socket"
     * @param option3
     */
    int init(
        const std::string& option,
        const std::string& option2,
        const void *option3
    ) override;

    void done() override;

    void onSend(
        const void *dispatcher,
        const MessageQueueItem *item,
        int code
    ) override;

    const char *name() override;
};

EXPORT_SHARED_C_FUNC AppBridge* makeBridge3();

#endif //TLNS_STDOUT_BRIDGE_H
