#ifndef TASK_SOCKET_H_
#define TASK_SOCKET_H_ 1

#include <cinttypes>
#include "lorawan/task/task-platform.h"
#include "lorawan/storage/network-identity.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#else
#include <netinet/in.h>
#endif

#include "lorawan/storage/gateway-identity.h"

class ProtoGwParser;

enum ENUM_SOCKET_ACCEPT {
    SA_NONE,        ///< socket do not require accept()
    SA_ACCEPT_REQUIRE,     ///< socket require accept()
    SA_ACCEPTED,    ///< socket require accept() and already accepted
    SA_TIMER,       ///< timer
    SA_EVENTFD      ///< reserved
};

/**
 * TaskSocket is read/write interface for incoming/outgoing messages.
 * Network server (MessageTaskDispatcher) open TaskSocket on create time to read messages from the gateway.
 * MessageTaskDispatcher close TaskSocket on destroy time
 * When TaskSocket receive message, it returns to the MessageTaskDispatcher.
 * Then MessageTaskDispatcher try to parse message and send reply- write to TaskSocket.
 *
 * Inherited classes:
 *  TaskUDPSocket       Task UDP socket
 *  TaskUnixSocket      Task Unix domain socket
 *  TaskAcceptedSocket  Task TCP socket after accept()
 * Task*ControlSocket classes intend for receive messages from the Network server
 * with request to stop the server
 *  TaskUDPControlSocket
 *  TaskUnixControlSocket
 *
 * MessageTaskDispatcher
 *   add socket: dispatcher.sockets.push_back(new TaskUnixSocket(FILE_NAME_UNIX_SOCKET));
 *   assign control socket: dispatcher.setControlSocket(new TaskUnixControlSocket(FILE_NAME_UNIX_SOCKET));
 */
class TaskSocket {
public:
    SOCKET sock;
    ENUM_SOCKET_ACCEPT socketAccept;        ///< Does socket require accept()?
    int lastError;                          ///< last error code. 0- success
    bool customWrite;                       ///< if true, dispatcher must call customWriteSocket() instead of write to socket
    TaskSocket();
    TaskSocket(
        ENUM_SOCKET_ACCEPT accept
    );
    TaskSocket(
        SOCKET socket,
        ENUM_SOCKET_ACCEPT accept
    );
    /**
     * Inherited class must implement openSocket() and closeSocket()
     * @return socket number, -1 if fails
     */
    virtual SOCKET openSocket() = 0;
    /**
     * Inherited class must implement openSocket() and closeSocket()
     * @return socket number, -1 if fails
     */
    virtual void closeSocket() = 0;
    /**
     * Send downlink message to the end-device if customWrite == true
     * @param networkIdentity device identity
     * @param data message buffer
     * @param size buffer size
     * @param proto gateway protocol
     */
    virtual void customWriteSocket(
        const NetworkIdentity *networkIdentity,
        const void* data,
        size_t size,
        ProtoGwParser *proto
    );
    virtual ~TaskSocket();
    std::string toString() const;
    std::string toJsonString() const;
};

/**
 * When socket has been accepted TaskSocketPreNAcceptedSocket class keeps accepted socket with "originator" TaskSocket
 * together
 */
class TaskSocketPreNAcceptedSocket {
public:
    TaskSocket *taskSocket;
    SOCKET acceptedSocket;
    TaskSocketPreNAcceptedSocket();
    TaskSocketPreNAcceptedSocket(
        TaskSocket *taskSocket,
        SOCKET acceptedSocket
    );
};

class GatewayPingTimeNSocket {
public:
    TaskSocket *taskSocket;
    TASK_TIME tim;
    GatewayPingTimeNSocket();
    GatewayPingTimeNSocket(
        TaskSocket *taskSocket
    );
};

#endif
