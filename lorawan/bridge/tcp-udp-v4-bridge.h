#ifndef TLNS_TCP_UDP_V4_BRIDGE_H
#define TLNS_TCP_UDP_V4_BRIDGE_H

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
