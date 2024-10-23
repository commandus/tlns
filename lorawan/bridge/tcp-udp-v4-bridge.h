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
    std::string addrAndPort;
    std::thread *thread;
    bool running;
    bool stopped;
    int openSockets();
    void closeSockets();
    void start();
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

    void init(
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
