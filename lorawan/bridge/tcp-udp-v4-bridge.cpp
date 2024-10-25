#include <iostream>
#include <cstring>

#define DEF_PORT                    4250
#define DEF_MAX_TCP_CONNECTIONS     1024
#define DEF_MAX_UDP_CONNECTIONS     1024
// UDP clients must ping every hour
#define DEF_MAX_UDP_CONNECTION_EXPIRATION_SECONDS   3600
#define DEF_ON_PAYLOAD_SOCKET_PATH  "/tmp/tcp-udp-v4-bridge.socket"

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <WS2tcpip.h>
#include <Winsock2.h>
#define close(x) closesocket(x)
#define write(sock, b, sz) ::send(sock, b, sz, 0)
// #define inet_pton InetPtonA
#else
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <functional>

#define INVALID_SOCKET  (-1)
#endif

#include <fcntl.h>
#include <sstream>

#include "lorawan/bridge/tcp-udp-v4-bridge.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/ip-address.h"
#include "lorawan/lorawan-error.h"

static const char *APP_BRIDGE_NAME = "tcp-udp-v4-app-bridge";

TcpUdpV4Bridge::TcpUdpV4Bridge()
    : tcpListenSocket(INVALID_SOCKET), udpSocket(INVALID_SOCKET),
      running(false), stopped(true), thread(nullptr), onPayloadSocketPath(DEF_ON_PAYLOAD_SOCKET_PATH)
{

}

int TcpUdpV4Bridge::openOnPayloadSocket()
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    return INVALID_SOCKET;
#else
    onPayloadSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (onPayloadSocket <= 0)
        return ERR_CODE_SOCKET_CREATE;
    struct sockaddr_un sunAddr;
    memset(&sunAddr, 0, sizeof(struct sockaddr_un));

    // Allow socket descriptor to be reusable
    int on = 1;
    int rc = setsockopt(onPayloadSocket, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
    if (rc < 0) {
        closeOnPayloadSocket();
        return ERR_CODE_SOCKET_CREATE;
    }
    // Set socket to be nonblocking
    int flags = fcntl(onPayloadSocket, F_GETFL, 0);
    fcntl(onPayloadSocket, F_SETFL, flags | O_NONBLOCK);
    // make sure
    rc = ioctl(onPayloadSocket, FIONBIO, (char *) &on);
    if (rc < 0) {
        closeOnPayloadSocket();
        return ERR_CODE_SOCKET_CREATE;
    }
    // Bind socket to socket name
    sunAddr.sun_family = AF_UNIX;
    strncpy(sunAddr.sun_path, onPayloadSocketPath.c_str(), sizeof(sunAddr.sun_path) - 1);
    int r = bind(onPayloadSocket, (const struct sockaddr *) &sunAddr, sizeof(struct sockaddr_un));
    if (r < 0) {
        closeOnPayloadSocket();
        return ERR_CODE_SOCKET_BIND;
    }
    // Prepare for accepting connections. The backlog size is set to 20. So while one request is being processed other requests can be waiting.
    r = listen(onPayloadSocket, 20);
    if (r < 0) {
        closeOnPayloadSocket();
        return ERR_CODE_SOCKET_LISTEN;
    }
    return CODE_OK;
#endif
}

int TcpUdpV4Bridge::openSockets()
{
    int r = openOnPayloadSocket();
    if (r < 0)
        return r;

    struct sockaddr_in srvAddr;
    bzero(&srvAddr, sizeof(srvAddr));
    srvAddr.sin_family = AF_INET;

    std::string a;
    uint16_t port;
    splitAddress(a, port, addrAndPort);
    if (a.empty() || a == "*")
        srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        r = inet_pton(AF_INET, a.c_str(), &srvAddr.sin_addr);
    if (r)
        return ERR_CODE_SOCKET_ADDRESS;
    if (port == 0)
        port = DEF_PORT;
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
    return CODE_OK;
}

void TcpUdpV4Bridge::closeOnPayloadSocket()
{
    unlink(onPayloadSocketPath.c_str());
    if (onPayloadSocket == INVALID_SOCKET)
        return;
    close(onPayloadSocket);
    unlink(onPayloadSocketPath.c_str());
    onPayloadSocket = INVALID_SOCKET;
}

void TcpUdpV4Bridge::closeSockets()
{
    if (tcpListenSocket != INVALID_SOCKET)
        close(tcpListenSocket);
    if (udpSocket != INVALID_SOCKET)
        close(udpSocket);
    closeOnPayloadSocket();
}

int TcpUdpV4Bridge::start()
{
    int r = openSockets();
    if (r)
        return r;

    running = true;
    stopped = false;
    thread = new std::thread(std::bind(&TcpUdpV4Bridge::run, this));
    thread->detach();
    return CODE_OK;
}

void TcpUdpV4Bridge::stop()
{
    running = false;
    while(!stopped) {
        sleep(1);
    }
    closeSockets();
}

class InAddrPort {
public:
    uint16_t port;
    uint32_t addr;
    explicit InAddrPort(
        const struct sockaddr_in &a
    )
        : port(a.sin_port), addr(a.sin_addr.s_addr)
    {
    }
    bool operator<(
         const InAddrPort &rhs
    ) const {
        if (addr < rhs.addr)
            return true;
        if (addr > rhs.addr)
            return false;
        return (port < rhs.port);
    }


};

class UdpClient {
public:
    time_t t;
    struct sockaddr_in addr;
    UdpClient()
        : t(0)
    {
        addr.sin_port = 0;
        addr.sin_addr.s_addr = 0;
    }

    UdpClient(const UdpClient &value)
        : t(value.t), addr(value.addr)
    {
    }

    explicit UdpClient(
        const struct sockaddr_in &aAddr
    )
        : t(time(nullptr)), addr(aAddr)
    {

    }
};

class UdpClients {
private:
    size_t maxSize;
    int expirationSeconds;
public:
    std::map<InAddrPort, UdpClient> udpClientAddress;
    UdpClients(size_t aMaxSize, int aExpirationSeconds)
        : maxSize(aMaxSize), expirationSeconds(aExpirationSeconds)
    {

    }

    void cleanExpired() {
        time_t expTime(time(nullptr));
        expTime -= expirationSeconds;
        for (auto a(udpClientAddress.begin()); a != udpClientAddress.end();) {
            if (a->second.t > expTime) {
                a = udpClientAddress.erase(a);
                continue;
            }
            a++;
        }
    }

    bool push(const struct sockaddr_in &a) {
        // check limits
        if (udpClientAddress.size() >= maxSize) {
            cleanExpired();
            if (udpClientAddress.size() >= maxSize)
                return false;
        }
        udpClientAddress[InAddrPort(a)] = UdpClient(a);
        return true;
    }

    /**
     * Send to all UDP clients
     * @param socket UDP socket
     * @param message message buffer
     * @param size message buffer size
     * @return count of successfully sent messages
     */
    size_t send2all(
        SOCKET socket,
        void *message,
        size_t size
    ) {
        size_t count = 0;
        for (auto a(udpClientAddress.begin()); a != udpClientAddress.end();) {
            ssize_t r = sendto(socket, (const char *) message, size, 0,
                   (struct sockaddr *) &a->second.addr, sizeof(struct sockaddr_in));
            if (r < 0) {
                // smth wrong, remove client address from the list
                a = udpClientAddress.erase(a);
                continue;
            }
            a++;    // next client
            count++;
        }
        return count;
    }

};

void TcpUdpV4Bridge::run()
{
    // clear the descriptor set
    fd_set rset;
    FD_ZERO(&rset);
    struct sockaddr_in clientAddr;
    std::vector <SOCKET> tcpClientSockets;

    struct timeval selectTimeout;
    selectTimeout.tv_sec = 1;
    selectTimeout.tv_usec = 0;

    UdpClients udpClients(DEF_MAX_UDP_CONNECTIONS, DEF_MAX_UDP_CONNECTION_EXPIRATION_SECONDS);
    while (running) {
        // add listen TCP socket and UDP socket
        FD_SET(tcpListenSocket, &rset);
        FD_SET(udpSocket, &rset);
        FD_SET(onPayloadSocket, &rset);

        // determine max socket number
        SOCKET maxSocketPlus1 = (tcpListenSocket > udpSocket ? tcpListenSocket : udpSocket);
        if (onPayloadSocket > maxSocketPlus1)
            maxSocketPlus1 = maxSocketPlus1;

        for (auto s : tcpClientSockets) {
            // add client's TCP socket
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
            SOCKET clientConnectionSocket = accept(tcpListenSocket, (struct sockaddr*) &clientAddr, &len);
            if (clientConnectionSocket >= 0) {
                if (tcpClientSockets.size() < DEF_MAX_TCP_CONNECTIONS)
                    tcpClientSockets.push_back(clientConnectionSocket);
                else
                    close(clientConnectionSocket);
            }
        }

        char buffer[4096];

        if (FD_ISSET(onPayloadSocket, &rset)) {
            // payload and onSent event
            ssize_t n = read(onPayloadSocket, buffer, sizeof(buffer));
            if (n > 0) {
                // write to all connected TCP clients
                for (auto s: tcpClientSockets) {
                    write(s, (const char *) buffer, n);
                }
                // send to UDP clients
                udpClients.send2all(udpSocket, buffer, n);
            }
        }

        // if udp socket is readable receive the message.
        if (FD_ISSET(udpSocket, &rset)) {
            len = sizeof(clientAddr);
            ssize_t n = recvfrom(udpSocket, buffer, sizeof(buffer), 0,
                                 (struct sockaddr*) &clientAddr, &len);
            if (n < 0) {
                // error, nothing to do
            } else {
                udpClients.push(clientAddr);
            }
        }
        // client's tcp connections
        for (auto s(tcpClientSockets.begin()); s != tcpClientSockets.end();) {
            if (FD_ISSET(*s, &rset)) {
                ssize_t n = read(*s, buffer, sizeof(buffer));
                if (n < 0) {
                    // error occurs, close socket.
                    close(*s);
                    s = tcpClientSockets.erase(s);
                    continue;
                }
            }
            // next socket
            s++;
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
        if (onPayloadSocket >= 0) {
            std::string s = messageItem->toJsonString();
            // remove "{"
            s.erase(0, 1);
            // add extra flags
            std::stringstream ss;
            ss << "{\"payloadDecoded\": " << (decoded ? "true" : "false")
                << ", \"payloadMicMatched\": " << (micMatched ? "true" : "false")
                << ", " << s;
            s = ss.str();
            write(onPayloadSocket, (const char *) s.c_str(), s.size());
        }
    }
}

int TcpUdpV4Bridge::init(
    const std::string& option,
    const std::string& option2,
    const void *option3
)
{
    addrAndPort = option;
    if (!option2.empty())
        onPayloadSocketPath = option2;
    return start();
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
    if (item) {
        if (onPayloadSocket >= 0) {
            std::string s = item->toJsonString();
            // remove "{"
            s.erase(0, 1);
            // add extra flags
            std::stringstream ss;
            ss << "{\"sendingResultCode\": " << code
               << ", " << s;
            s = ss.str();
            write(onPayloadSocket, (const char *) s.c_str(), s.size());
        }
    }

}

const char *TcpUdpV4Bridge::name()
{
    return APP_BRIDGE_NAME;
}

EXPORT_SHARED_C_FUNC AppBridge* makeBridge3()
{
    return new TcpUdpV4Bridge;
}
