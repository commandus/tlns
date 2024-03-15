#ifndef MESSAGE_TASK_DISPATCHER_H
#define MESSAGE_TASK_DISPATCHER_H

#include <thread>
#include <condition_variable>

#include "lorawan/task/message-queue.h"
#include "lorawan/task/task-response.h"
#include "lorawan/helper/ip-address.h"

#include "lorawan/proto/gw/gw.h"

typedef void(*OnPushDataProc)(
        MessageTaskDispatcher* dispatcher,
        GwPushData &item
);

typedef void(*OnPullRespProc)(
        MessageTaskDispatcher* dispatcher,
        GwPullResp &item
);

typedef void(*OnTxpkAckProc)(
        MessageTaskDispatcher* dispatcher,
        ERR_CODE_TX code
);

class MessageTaskDispatcher;
class TaskSocket;

typedef int(*TaskProc)(
    MessageTaskDispatcher *env,
    TaskSocket *socket,
    const struct sockaddr *src,
    TASK_TIME receivedTime,
    const char *buffer,
    size_t size
);

class TaskSocket {
public:
    SOCKET sock;
    in_addr_t addr;
    uint16_t port;
    int lastError;
    /**
     * @param addr ""- any interface, "localhost"- localhost otherwise- address
     * @param port port number
     */
    TaskSocket(in_addr_t aAddr, uint16_t port);
    /**
     * Open UDP socket for listen
     * @return -1 if fail
     */
    SOCKET openUDPSocket();
    void closeSocket();
    // virtual int onData(const char *buffer, size_t size) = 0;
    virtual ~TaskSocket();
};

class ProtoGwParser;

class MessageTaskDispatcher {
private:
    // socket to send 'commands'
    SOCKET clientControlSocket;
protected:
    TaskResponse *taskResponse;
    // main loop thread
    std::thread *thread;
    // wait for thread loop done
    mutable std::condition_variable loopExit;

    bool openSockets();
    /**
     * close all sockets
     */
    void closeSockets();
    /**
     * erase all sockets
     */
    void clearSockets();
public:
    ProtoGwParser* parser;  ///< protocol parser
    // message queue
    MessageQueue queue;
    // task socket array
    std::vector<TaskSocket*> sockets;
    bool running;    ///< true- loop thread is running

    OnPushDataProc onPushData;
    OnPullRespProc onPullResp;
    OnTxpkAckProc onTxPkAck;

    int runner();

    MessageTaskDispatcher();
    MessageTaskDispatcher(const MessageTaskDispatcher &value);
    void setPorts(uint16_t control);
    virtual ~MessageTaskDispatcher();

    void response(MessageQueueItem *item);
    void setResponse(TaskResponse *receiver);

    void send(const void *buffer, size_t size);
    void send(char cmd);
    void send(const std::string& cmd);

    bool start();
    void stop();

    ssize_t sendACK(
        const TaskSocket *taskSocket,
        const sockaddr &destAddr,
        socklen_t destAddrLen,
        const char *packet,
        ssize_t packetSize
    );
};

#endif
