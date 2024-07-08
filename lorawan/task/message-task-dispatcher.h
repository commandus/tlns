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
#include "lorawan/regional-parameters/regional-parameter-channel-plan.h"
#include "task-timer-socket.h"

typedef void(*OnPushDataProc)(
    MessageTaskDispatcher* dispatcher,
    SEMTECH_PROTOCOL_METADATA_RX metadata,
    void *radioPacket,
    size_t size
);

typedef bool (*OnReceiveRawData)(
    MessageTaskDispatcher* dispatcher,
    const char *buffer,
    size_t bufferSize,
    TASK_TIME receivedTime
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

typedef void(*OnDestroyProc)(
    MessageTaskDispatcher* dispatcher
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

/**
 * MessageTaskDispatcher receive messages from the one or more TaskSocket
 */
class MessageTaskDispatcher {
private:
    std::mutex queueMutex;
    TaskSocket *controlSocket;
    TaskTimerSocket *timerSocket;
    /**
     * Set descriptor set
     * @param retValReadSet
     * @return max socket file descriptor number plus 1
     */
    int getMaxDescriptor1(
        fd_set &retValReadSet
    );
protected:
    TaskResponse *taskResponse;
    std::thread *thread;    ///< main loop thread
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
    MessageQueue queue;     ///< message queue
    std::vector<TaskSocket*> sockets;   ///< task socket array
    bool running;    ///< true- loop thread is running

    OnReceiveRawData onReceiveRawData;
    OnPushMessageQueueItem onPushData;
    OnPullRespProc onPullResp;
    OnTxpkAckProc onTxPkAck;
    OnDestroyProc onDestroy;

    ProtoGwParser* parser;
    const RegionalParameterChannelPlan *regionalPlan;

    int run();

    MessageTaskDispatcher();
    MessageTaskDispatcher(const MessageTaskDispatcher &value);
    virtual ~MessageTaskDispatcher();

    void response(MessageQueueItem *item);
    void setResponse(TaskResponse *receiver);

    void send(const void *buffer, size_t size);
    void send(char cmd);
    void send(const std::string& cmd);

    void start();
    void stop();

    ssize_t sendACK(
        const TaskSocket *taskSocket,
        const sockaddr &destAddr,
        socklen_t destAddrLen,
        const char *packet,
        ssize_t packetSize
    );

    ssize_t sendConfirm(
        const TaskSocket *taskSocket,
        const sockaddr &destAddr,
        socklen_t destAddrLen
    );

    void setControlSocket(
        TaskSocket *socket
    );

    void pushData(GwPushData &pushData, TASK_TIME point);

    void setParser(ProtoGwParser *parser);

    void setRegionalParameterChannelPlan(
        const RegionalParameterChannelPlan *aRegionalPlan
    );

    bool sendQueue();

    void updateTimer(TASK_TIME now);

    void prepareSendConfirmation(
        const DEVADDR *addr,
        TASK_TIME receivedTime
    );
};

#endif
