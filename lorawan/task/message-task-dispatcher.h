#ifndef MESSAGE_TASK_DISPATCHER_H
#define MESSAGE_TASK_DISPATCHER_H

#include <thread>
#include <condition_variable>

#include "lorawan/task/task-platform.h"
#include "lorawan/task/message-queue.h"
#include "lorawan/task/task-response.h"
#include "lorawan/helper/ip-address.h"
#include "lorawan/proto/gw/gw.h"
#include "lorawan/proto/gw/parse-result.h"
#include "lorawan/task/task-timer-socket.h"
#include "lorawan/regional-parameters/regional-parameter-channel-plan.h"
#include "lorawan/storage/client/direct-client.h"
#include "lorawan/bridge/app-bridge.h"
#include "lorawan/storage/client/device-best-gateway-direct-client.h"

typedef void(*OnPushDataProc)(
    MessageTaskDispatcher* dispatcher,
    const TaskSocket *taskSocket,
    const sockaddr &addr,
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

typedef void(*OnErrorProc)(
    MessageTaskDispatcher* dispatcher,
    int level,
    const std::string &module,
    int code,
    const std::string &message
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
    std::vector<AppBridge *> appBridges;

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
    DeviceBestGatewayDirectClient *deviceBestGatewayClient;
    MessageQueue queue;     ///< message queue
    std::vector<TaskSocket*> sockets;   ///< task socket array
    bool running;    ///< true- loop thread is running

    OnReceiveRawData onReceiveRawData;
    OnPushMessageQueueItem onPushData;
    OnPullRespProc onPullResp;
    OnTxpkAckProc onTxPkAck;
    OnDestroyProc onDestroy;
    OnErrorProc onError;

    std::vector<ProtoGwParser*> parsers;
    const RegionalParameterChannelPlan *regionalPlan;
    DirectClient *identityClient;

    int run();

    MessageTaskDispatcher();
    MessageTaskDispatcher(const MessageTaskDispatcher &value);
    virtual ~MessageTaskDispatcher();

    void response(MessageQueueItem *item);
    void setResponse(TaskResponse *receiver);

    void send(
        const void *buffer,
        size_t size
    );
    void send(
        char cmd
    );
    void send(
        const std::string& cmd
    );

    void start();
    void stop();

    ssize_t sendACK(
        const TaskSocket *taskSocket,
        const sockaddr &destAddr,
        socklen_t destAddrLen,
        const char *packet,
        ssize_t packetSize,
        ProtoGwParser *parser
    );

    ssize_t sendConfirm(
        const TaskSocket *taskSocket,
        const sockaddr &destAddr,
        socklen_t destAddrLen
    );

    void setControlSocket(
        TaskSocket *socket
    );

    void pushData(
        const TaskSocket *taskSocket,
        const sockaddr &addr,
        GwPushData &pushData,
        const TASK_TIME &receivedTime,
        ProtoGwParser *pParser
    );

    void addParser(
        ProtoGwParser *aParser
    );

    void setRegionalParameterChannelPlan(
        const RegionalParameterChannelPlan *aRegionalPlan
    );

    void sendQueue(
        TASK_TIME now,
        uint16_t token
    );

    bool isTimeProcessQueueOrSetTimer(
        TASK_TIME now
    );

    void prepareSendConfirmation(
        const DEVADDR *addr,
        const sockaddr &sockAddr,
        TASK_TIME receivedTime
    );

    void cleanupOldMessages(
        TASK_TIME now
    );

    /**
     * Check is gateway in the list of service
     * @param parsedMsg parsed message (push or pull response)
     * @param taskSocket socket received from
     * @param srcSockAddr source address
     * @return Return 0 if message received from known gateway. Return <0 if message to reject
     */
    int validateGatewayAddress(
        const ParseResult &parsedMsg,
        const TaskSocket *taskSocket,
        const sockaddr &srcSockAddr
    );

    /**
     * Identity client requests identity service for EUI and keys
     * @param aIdentityClient
     */
    void setIdentityClient(
        DirectClient *aIdentityClient
    );

    /** Bridge to application service(s)
     *
     * @param appBridge application service bridge instance
     */
    void addAppBridge(
        AppBridge *appBridge
    );
    /**
     * Send payload to all registered application services
     * @param item
     */
    void sendPayloadOverBridge(
        MessageQueueItem *item
    );

    /**
     * Assign service store best gateway for device
     * @param aClient pointer to DeviceBestGatewayDirectClient object
     */
    void setDeviceBestGatewayClient(
        DeviceBestGatewayDirectClient *aClient
    );

    /**
     * app bridges counter
     * @return count of app bridges
     */
    size_t bridgeCount() const;

    /**
     * Send payload and/or FOpts to the end-device
     * @param tim  time to send. If 0 or less than current time, it is time to send.
     * @param addr address of the end-device
     * @param payload payload, 0..255 bytes, can be NULL
     * @param fopts FOpts, MAC commands, 0..15 bytes, can be NULL
     * @param fPort 0- FOpts in the payload, 1..255- user defined payload
     * @param payloadSize 0..255
     * @param foptsSize 0..15
     * @return 0- success
     */
    int sendDownlink(
        const TASK_TIME &tim,
        const DEVADDR &addr,
        void *payload,
        void *fopts,
        uint8_t fPort,
        uint8_t payloadSize,
        uint8_t foptsSize
    );
};

#endif
