#include <iostream>
#include <cstring>

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <WS2tcpip.h>
#include <Winsock2.h>
#define close(x) closesocket(x)
#define write(sock, b, sz) ::send(sock, b, sz, 0)
// #define inet_pton InetPtonA
#else
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <functional>

#define INVALID_SOCKET  (-1)
#endif


#include "lorawan/bridge/tcp-udp-v4-bridge.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/ip-address.h"
#include "lorawan/lorawan-error.h"

static const char *APP_BRIDGE_NAME = "tcp-udp-v4-app-bridge";

TcpUdpV4Bridge::TcpUdpV4Bridge()
    : tcpListenSocket(INVALID_SOCKET), udpSocket(INVALID_SOCKET),
      running(false), stopped(true), thread(nullptr)
{

}

int TcpUdpV4Bridge::openSockets()
{
    struct sockaddr_in srvAddr;
    bzero(&srvAddr, sizeof(srvAddr));
    srvAddr.sin_family = AF_INET;

    std::string a;
    uint16_t port;
    int r = 0;
    splitAddress(a, port, addrAndPort);
    if (a.empty() || a == "*")
        srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        r = inet_pton(AF_INET, a.c_str(), &srvAddr.sin_addr);
    if (r)
        return ERR_CODE_SOCKET_ADDRESS;
    srvAddr.sin_port = htons(port);

    // binding server addr structure to tcpListenSocket
    // create TCP socket
    tcpListenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpListenSocket == INVALID_SOCKET)
        return ERR_CODE_SOCKET_CREATE;

    if (bind(tcpListenSocket, (struct sockaddr*) &srvAddr, sizeof(srvAddr)) != 0) {
        close(tcpListenSocket);
        tcpListenSocket = INVALID_SOCKET;
        return ERR_CODE_SOCKET_BIND;
    }
    if (listen(tcpListenSocket, 10) != 0) {
        close(tcpListenSocket);
        tcpListenSocket = INVALID_SOCKET;
        return ERR_CODE_SOCKET_LISTEN;
    }
    // create UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (tcpListenSocket == INVALID_SOCKET) {
        close(tcpListenSocket);
        tcpListenSocket = INVALID_SOCKET;
        return ERR_CODE_SOCKET_CREATE;
    }
    // binding server addr structure to udp sockfd
    if (bind(udpSocket, (struct sockaddr*) &srvAddr, sizeof(srvAddr)) != 0) {
        close(tcpListenSocket);
        close(udpSocket);
        tcpListenSocket = INVALID_SOCKET;
        udpSocket = INVALID_SOCKET;
        return ERR_CODE_SOCKET_BIND;

    }
    return 0;
}

void TcpUdpV4Bridge::closeSockets()
{
    if (tcpListenSocket != INVALID_SOCKET)
        close(tcpListenSocket);
    if (udpSocket != INVALID_SOCKET)
        close(udpSocket);
}

void TcpUdpV4Bridge::start()
{
    openSockets();

    running = true;
    stopped = false;
    thread = new std::thread(std::bind(&TcpUdpV4Bridge::run, this));
    thread->detach();
}

void TcpUdpV4Bridge::stop()
{
    running = false;
    while(!stopped) {
        sleep(1);
    }
    closeSockets();
}

void TcpUdpV4Bridge::run()
{
    // clear the descriptor set
    fd_set rset;
    FD_ZERO(&rset);
    struct sockaddr_in clientAddr;

    std::vector <SOCKET> tcpClientSockets;
    const char* message = "Hi";

    struct timeval selectTimeout;
    selectTimeout.tv_sec = 1;
    selectTimeout.tv_usec = 0;
    while (running) {
        FD_SET(tcpListenSocket, &rset);
        FD_SET(udpSocket, &rset);
        SOCKET maxSocketPlus1 = (tcpListenSocket > udpSocket ? tcpListenSocket : udpSocket);
        for (auto s : tcpClientSockets) {
            FD_SET(s, &rset);
            if (s > maxSocketPlus1)
                maxSocketPlus1 = s;
        }
        maxSocketPlus1++;

        // select the ready descriptor
        int socketsReady = select(maxSocketPlus1, &rset, nullptr, nullptr, &selectTimeout);
        if (socketsReady <= 0)
            continue;
        // if tcp socket is readable then handle
        socklen_t len;
        // it by accepting the connection
        if (FD_ISSET(tcpListenSocket, &rset)) {
            len = sizeof(clientAddr);
            SOCKET connfd = accept(tcpListenSocket, (struct sockaddr*) &clientAddr, &len);
            if (connfd) {
                tcpClientSockets.push_back(connfd);
            }
            close(connfd);
        }
        char buffer[300];
        // if udp socket is readable receive the message.
        if (FD_ISSET(udpSocket, &rset)) {
            len = sizeof(clientAddr);
            ssize_t n = recvfrom(udpSocket, buffer, sizeof(buffer), 0,
                                 (struct sockaddr*) &clientAddr, &len);
            sendto(udpSocket, (const char*) message, sizeof(*message), 0,
                   (struct sockaddr*)&clientAddr, sizeof(clientAddr));
        }
        // tcp
        for (auto s : tcpClientSockets) {
            if (FD_ISSET(s, &rset)) {
                read(s, buffer, sizeof(buffer));
                write(s, (const char*) message, sizeof(*message));
            }
        }
    }
    stopped = true;
}

void TcpUdpV4Bridge::onPayload(
    const void *dispatcher,
    const MessageQueueItem *messageItem,
    bool decoded,
    bool micMatched
)
{
    if (messageItem) {
        if (decoded)
            std::cout << messageItem->toString() << std::endl;
    }
}

void TcpUdpV4Bridge::init(
    const std::string& option,
    const std::string& option2,
    const void *option3
)
{
    addrAndPort = option;
    start();
}

void TcpUdpV4Bridge::done()
{
    stop();
}

void TcpUdpV4Bridge::onSend(
    const void *dispatcher,
    const MessageQueueItem *item,
    int code
)
{
    if (item)
        std::cerr << "Sent " << item->toString() << " with code " << code << std::endl;
}

const char *TcpUdpV4Bridge::name()
{
    return APP_BRIDGE_NAME;
}

EXPORT_SHARED_C_FUNC AppBridge* makeBridge3()
{
    return new TcpUdpV4Bridge;
}
