#ifndef MESSAGE_TASK_DISPATCHER_H
#define MESSAGE_TASK_DISPATCHER_H

#include <thread>
#include <condition_variable>

#include "lorawan/task/task-platform.h"
#include "lorawan/task/message-queue.h"
#include "lorawan/task/task-response.h"
#include "lorawan/helper/ip-address.h"
#include "lorawan/proto/gw/gw.h"
#include "lorawan/task/task-socket.h"

typedef void(*OnPushDataProc)(
    MessageTaskDispatcher* dispatcher,
    SEMTECH_PROTOCOL_METADATA_RX metadata,
    void *radioPacket,
    size_t size
);

typedef void(*OnPushMessageQueueItem)(
    MessageTaskDispatcher* dispatcher,
    MessageQueueItem *item
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

typedef int(*TaskProc)(
    MessageTaskDispatcher *env,
    TaskSocket *socket,
    const struct sockaddr *src,
    TASK_TIME receivedTime,
    const char *buffer,
    size_t size
);

class ProtoGwParser;

class MessageTaskDispatcher {
private:
    std::mutex queueMutex;
    TaskSocket *controlSocket;
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

    OnPushMessageQueueItem onPushData;
    OnPullRespProc onPullResp;
    OnTxpkAckProc onTxPkAck;

    int runner();

    MessageTaskDispatcher();
    MessageTaskDispatcher(const MessageTaskDispatcher &value);
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

    void setControlSocket(
        TaskSocket *socket
    );

    /**
     * Set descriptor set
     * @param retVal
     * @return
     */
    int getMaxDescriptor1(fd_set &retVal);

    void pushData(GwPushData &pushData);
};

#endif
