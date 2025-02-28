#include <iostream>
#include <functional>

#define DEF_PORT                    4250
#define DEF_MAX_CONNECTIONS         1010
// UDP clients must ping every hour
#define DEF_MAX_UDP_CONNECTION_EXPIRATION_SECONDS   3600
#define DEF_ON_PAYLOAD_SOCKET_PATH  "/tmp/tcp-udp-v4-bridge.socket"

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <WS2tcpip.h>
#include <io.h>
#include <Winsock2.h>
#define sleep(x) Sleep(x)
#define unlink(x)
#define write _write
#define read _read
#define close(x) closesocket(x)
#define MSG_NOSIGNAL    0
#else
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define INVALID_SOCKET  (-1)
#endif

#include <fcntl.h>
#include <sstream>

#include "lorawan/bridge/tcp-udp-v4-bridge.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/ip-address.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/proto/payload2device/payload2device-parser.h"

static const char *APP_BRIDGE_NAME = "tcp-udp-v4-app-bridge";
#define BUFFER_SIZE 4096

TcpUdpV4Bridge::TcpUdpV4Bridge()
    : tcpListenSocket(INVALID_SOCKET), udpSocket(INVALID_SOCKET),
    onPayloadListenSocket(INVALID_SOCKET), onPayloadAcceptedSocket(INVALID_SOCKET), onPayloadClientSocket(INVALID_SOCKET),
    running(false), stopped(true), thread(nullptr), onPayloadSocketPath(DEF_ON_PAYLOAD_SOCKET_PATH)
{

}

TcpUdpV4Bridge::~TcpUdpV4Bridge()
{
    stop();
}

int TcpUdpV4Bridge::openOnPayloadSocket()
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    onPayloadListenSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (onPayloadListenSocket == INVALID_SOCKET)
        return ERR_CODE_SOCKET_CREATE;
    struct sockaddr_in sinAddr {AF_INET, 0 };

    // Allow socket descriptor to be reusable
    int on = 1;
    int rc = setsockopt(onPayloadListenSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    if (rc < 0) {
        closeOnPayloadSocket();
        return ERR_CODE_SOCKET_CREATE;
    }
    // Set socket to be nonblocking
    u_long onw = 1;
    rc = ioctlsocket(onPayloadListenSocket, FIONBIO, &onw);
    if (rc < 0) {
        closeOnPayloadSocket();
        return ERR_CODE_SOCKET_CREATE;
    }

    // Bind socket to socket name
    sinAddr.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
    sinAddr.sin_port = 0;   // TCP/IP stack assign random port number
    int r = bind(onPayloadListenSocket, (const struct sockaddr *) &sinAddr, sizeof(struct sockaddr_in));
    if (r < 0) {
        closeOnPayloadSocket();
        return ERR_CODE_SOCKET_BIND;
    }
    // get port number back
    int nameLen = sizeof(struct sockaddr_in);
    getsockname(onPayloadListenSocket, (sockaddr *) &sinAddr, &nameLen);

    // client socket
    onPayloadClientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    r = connect(onPayloadClientSocket, (const struct sockaddr *) &sinAddr, sizeof(struct sockaddr_in));
    if (r < 0) {
        closeOnPayloadSocket();
        return ERR_CODE_SOCKET_CONNECT;
    }
#else
    unlink(onPayloadSocketPath.c_str());
    onPayloadListenSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (onPayloadListenSocket == INVALID_SOCKET)
        return ERR_CODE_SOCKET_CREATE;
    struct sockaddr_un sunAddr { AF_UNIX,'\0'};

    // Allow socket descriptor to be reusable
    int on = 1;
    int rc = setsockopt(onPayloadListenSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    if (rc < 0) {
        closeOnPayloadSocket();
        return ERR_CODE_SOCKET_CREATE;
    }
    // Set socket to be nonblocking
    int flags = fcntl(onPayloadListenSocket, F_GETFL, 0);
    fcntl(onPayloadListenSocket, F_SETFL, flags | O_NONBLOCK);
    // make sure
    rc = ioctl(onPayloadListenSocket, FIONBIO, (char *) &on);
    if (rc < 0) {
        closeOnPayloadSocket();
        return ERR_CODE_SOCKET_CREATE;
    }
    // Bind socket to socket name
    strncpy(sunAddr.sun_path, onPayloadSocketPath.c_str(), sizeof(sunAddr.sun_path) - 1);
    int r = bind(onPayloadListenSocket, (const struct sockaddr *) &sunAddr, sizeof(struct sockaddr_un));
    if (r < 0) {
        closeOnPayloadSocket();
        return ERR_CODE_SOCKET_BIND;
    }
    // Prepare for accepting connections. The backlog size is set to 20. So while one request is being processed other requests can be waiting.
    r = listen(onPayloadListenSocket, 20);
    if (r < 0) {
        closeOnPayloadSocket();
        return ERR_CODE_SOCKET_LISTEN;
    }

    // client socket
    onPayloadClientSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    r = connect(onPayloadClientSocket, (const struct sockaddr *) &sunAddr, sizeof(struct sockaddr_un));
    if (r < 0) {
        closeOnPayloadSocket();
        return ERR_CODE_SOCKET_CONNECT;
    }
#endif
    return CODE_OK;
}

int TcpUdpV4Bridge::openSockets()
{
    int r = openOnPayloadSocket();
    if (r < 0)
        return r;
    std::string a;
    uint16_t port;
    splitAddress(a, port, addrAndPort);
    struct sockaddr_in srvAddr { AF_INET, 0, 0 };
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
    int on = 1;
#if defined(_MSC_VER) || defined(__MINGW32__)
    if (setsockopt(tcpListenSocket, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on)) < 0) {
        close(tcpListenSocket);
        tcpListenSocket = INVALID_SOCKET;
        return ERR_CODE_SOCKET_OPEN;
    }
#else
    if (setsockopt(tcpListenSocket, SOL_SOCKET,  SO_REUSEPORT, (char *)&on, sizeof(on)) < 0) {
        close(tcpListenSocket);
        tcpListenSocket = INVALID_SOCKET;
        return ERR_CODE_SOCKET_OPEN;
    }
#endif
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
    udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET) {
        close(tcpListenSocket);
        tcpListenSocket = INVALID_SOCKET;
        return ERR_CODE_SOCKET_CREATE;
    }
#if defined(_MSC_VER) || defined(__MINGW32__)
    if (setsockopt(udpSocket, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on)) < 0) {
        close(tcpListenSocket);
        tcpListenSocket = INVALID_SOCKET;
        return ERR_CODE_SOCKET_OPEN;
    }
#else
    if (setsockopt(udpSocket, SOL_SOCKET,  SO_REUSEPORT, (char *)&on, sizeof(on)) < 0) {
        close(tcpListenSocket);
        tcpListenSocket = INVALID_SOCKET;
        return ERR_CODE_SOCKET_OPEN;
    }
#endif
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
    if (onPayloadListenSocket != INVALID_SOCKET) {
        close(onPayloadListenSocket);
        unlink(onPayloadSocketPath.c_str());
        onPayloadListenSocket = INVALID_SOCKET;
    }

    if (onPayloadClientSocket != INVALID_SOCKET) {
        close(onPayloadClientSocket);
        onPayloadClientSocket = INVALID_SOCKET;
    }
}

void TcpUdpV4Bridge::closeSockets()
{
    if (tcpListenSocket != INVALID_SOCKET) {
        close(tcpListenSocket);
        tcpListenSocket = INVALID_SOCKET;
    }
    if (udpSocket != INVALID_SOCKET) {
        close(udpSocket);
        udpSocket = INVALID_SOCKET;
    }
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
    InAddrPort()
        : port(0), addr(0)
    {

    }

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
        : t(0), addr { AF_INET, 0, 0}
    {
    }

    UdpClient(
        const UdpClient &value
    )
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
    int expirationSeconds;
public:
    std::map<InAddrPort, UdpClient> udpClientAddress;
    explicit UdpClients(
        int aExpirationSeconds
    )
        : expirationSeconds(aExpirationSeconds)
    {

    }

    /**
     * Remove expired client addresses.
     * @param removeOldest force remove oldest client address if no expired clients found.
     * @return true if at least one client has been removed.
     */
    bool cleanExpired(bool removeOldest = false) {
        time_t expTime(time(nullptr));
        expTime -= expirationSeconds;
        time_t oldest = 0;
        InAddrPort oldestKey;
        bool removed = false;
        for (auto a(udpClientAddress.begin()); a != udpClientAddress.end();) {
            if (a->second.t > expTime) {
                removeOldest = false; // element deleted, so do not delete extra items
                a = udpClientAddress.erase(a);
                removed = true;
                continue;
            }
            if (removeOldest && (a->second.t > oldest)) {
                oldest = a->second.t;
                oldestKey = a->first;
            }
            a++;
        }
        // if expired items not found, remove oldest one
        if (removeOldest && (oldest != 0)) {
            auto it = udpClientAddress.find(oldestKey);
            if (it != udpClientAddress.end()) {
                udpClientAddress.erase(it);
                removed = true;
            }
        }
        return removed;
    }

    bool push(
        const struct sockaddr_in &a,
        bool replaceOldest
    ) {
        // check limits
        if (replaceOldest)
            if (!cleanExpired(true))
                return false;
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
            ssize_t r = (ssize_t) sendto(socket, (const char *) message, (int)size, 0,
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
    fd_set rset;
    struct sockaddr_in clientAddr {AF_INET, 0, 0 };
    struct timeval selectTimeout { 0, 0 };

    std::vector <SOCKET> tcpClientSockets;
    UdpClients udpClients(DEF_MAX_UDP_CONNECTION_EXPIRATION_SECONDS);
    while (running) {
        // set timeout value
        selectTimeout.tv_sec = 1;
        selectTimeout.tv_usec = 0;
        // clear the descriptor set
        FD_ZERO(&rset);
        // add listen TCP, UDP and UNIX sockets
        FD_SET(tcpListenSocket, &rset);
        FD_SET(udpSocket, &rset);
        FD_SET(onPayloadListenSocket, &rset);

        // determine max socket number
        SOCKET maxSocketPlus1 = (tcpListenSocket > udpSocket ? tcpListenSocket : udpSocket);
        if (onPayloadListenSocket > maxSocketPlus1)
            maxSocketPlus1 = maxSocketPlus1;

        if (onPayloadAcceptedSocket != INVALID_SOCKET) {
            FD_SET(onPayloadAcceptedSocket, &rset);
            if (onPayloadAcceptedSocket > maxSocketPlus1)
                maxSocketPlus1 = onPayloadAcceptedSocket;
        }

        // add accepted TCP sockets to the set
        for (auto s : tcpClientSockets) {
            // add client's TCP socket
            FD_SET(s, &rset);
            if (s > maxSocketPlus1)
                maxSocketPlus1 = s;
        }
        maxSocketPlus1++;

        // select the ready descriptor
        int socketsReady = select((int) maxSocketPlus1, &rset, nullptr, nullptr, &selectTimeout);
        if (socketsReady <= 0)
            continue;
        // if tcp socket is readable then handle
        socklen_t len;
        // it by accepting the connection
        if (FD_ISSET(tcpListenSocket, &rset)) {
            len = sizeof(clientAddr);
            SOCKET clientConnectionSocket = accept(tcpListenSocket, (struct sockaddr*) &clientAddr, &len);
            if (clientConnectionSocket >= 0) {
                if (tcpClientSockets.size() + udpClients.udpClientAddress.size() < DEF_MAX_CONNECTIONS) {
                    tcpClientSockets.push_back(clientConnectionSocket);
                } else
                    close(clientConnectionSocket);
            }
        }

        char buffer[BUFFER_SIZE];

        if (FD_ISSET(onPayloadListenSocket, &rset)) {
            // close previous socket if assigned
            if (onPayloadAcceptedSocket != INVALID_SOCKET)
                close(onPayloadAcceptedSocket);
            // accept
            len = sizeof(clientAddr);
            onPayloadAcceptedSocket = accept(onPayloadListenSocket, (struct sockaddr*) &clientAddr, &len);
        }

        if (FD_ISSET(onPayloadAcceptedSocket, &rset)) {
            // payload and onSent event
            ssize_t n = read((int) onPayloadAcceptedSocket, buffer, (int) sizeof(buffer));

            if (n > 0) {
                // write to all connected TCP clients
                // client's tcp connections
                for (auto s(tcpClientSockets.begin()); s != tcpClientSockets.end();) {
                    ssize_t nw = (ssize_t) send(*s, (const char *) buffer, (int) n, MSG_NOSIGNAL);
                    if (nw < 0) {
                        // error occurs, close socket.
                        close(*s);
                        s = tcpClientSockets.erase(s);
                        continue;
                    }
                    // next socket
                    s++;
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
                udpClients.push(clientAddr,
                    tcpClientSockets.size() + udpClients.udpClientAddress.size() >= DEF_MAX_CONNECTIONS);
                parseNsend2device((const char *) buffer, n);
            }
        }
        // client's tcp connections read
        for (auto s(tcpClientSockets.begin()); s != tcpClientSockets.end();) {
            if (FD_ISSET(*s, &rset)) {
                ssize_t n = read((int) *s, buffer, (int) sizeof(buffer));
                if (n < 0) {
                    // error occurs, close socket.
                    close(*s);
                    s = tcpClientSockets.erase(s);
                    continue;
                }
                parseNsend2device((const char *) buffer, n);
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
        if (onPayloadClientSocket >= 0) {
            std::string s = messageItem->toJsonString();
            // remove "{"
            s.erase(0, 1);
            // add extra flags
            std::stringstream ss;
            ss << "{\"payloadDecoded\": " << (decoded ? "true" : "false")
                << ", \"payloadMicMatched\": " << (micMatched ? "true" : "false")
                << ", " << s;
            s = ss.str();
            auto sz = s.size();
            // Invalid in WIndows- it does not work with sockets
            // ssize_t bytes = (ssize_t) write((SOCKET) onPayloadClientSocket, (const char *) s.c_str(), (int) sz);
            ssize_t bytes = (ssize_t) send(onPayloadClientSocket, (const char *) s.c_str(), (int) sz, 0);
            if (bytes < sz) {
                std::cerr << "Error write " << bytes << ", errno: " << errno << std::endl;
            }
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
        if (onPayloadClientSocket >= 0) {
            std::string s = item->toJsonString();
            // remove "{"
            s.erase(0, 1);
            // add extra flags
            std::stringstream ss;
            ss << "{\"sendingResultCode\": " << code
               << ", " << s;
            s = ss.str();
            write((SOCKET) onPayloadClientSocket, (const char *) s.c_str(), (int) s.size());
        }
    }

}

const char *TcpUdpV4Bridge::name()
{
    return APP_BRIDGE_NAME;
}

size_t TcpUdpV4Bridge::parseNsend2device(
    const char *expression,
    size_t size
)
{
    size_t r = 0;
    Payload2DeviceParser p;
    if (p.parse(expression, size) == PAYLOAD2DEVICE_COMMAND_SEND) {
        //
        for (auto &a : p.addresses) {
            TASK_TIME t;
            this->send2addr(t, a, (void *) p.payload.c_str(), (void *) p.fopts.c_str(), p.fport,
                (uint8_t ) p.payload.size(), (uint8_t) p.fopts.size());
            r++;
        }
    }
    return r;
}

EXPORT_SHARED_C_FUNC AppBridge* makeBridge3()
{
    return new TcpUdpV4Bridge;
}
