#if defined(_MSC_VER) || defined(__MINGW32__)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "message-task-dispatcher.h"

#include <algorithm>
#include <functional>
#include <csignal>
#include <cstring>
#include <iostream>
#include <sstream>

#include "lorawan/lorawan-error.h"
#include "lorawan/proto/gw/basic-udp.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-msg.h"
#include "lorawan/task/task-accepted-socket.h"
#include "lorawan/lorawan-date.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <Winsock2.h>

#define write(sock, b, sz) ::send(sock, b, sz, 0)
#define read(sock, b, sz) ::recv(sock, b, sz, 0)
typedef in_addr in_addr_t;
#else
#define INVALID_SOCKET (-1)
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/timerfd.h>
#endif

#define DEF_TIMEOUT_SECONDS 3
#define DEF_WAIT_QUIT_SECONDS 1
#define MAX_ACK_SIZE    8
#define MIN_TIMER_IN_MICROSECONDS   9000

MessageTaskDispatcher::MessageTaskDispatcher()
    : controlSocket(nullptr), timerSocket(new TaskTimerSocket), taskResponse(nullptr), threadUplink(nullptr),
    deviceBestGatewayClient(nullptr), regionalPlan(nullptr), identityClient(nullptr), runningUplink(false),
    onReceiveRawData(nullptr), onPushData(nullptr), onPullResp(nullptr), onTxPkAck(nullptr), onDestroy(nullptr),
    onError(nullptr), onStart(nullptr), onStop(nullptr), onGatewayPing(nullptr)
{
    queue.setDispatcher(this);
    sockets.push_back(timerSocket);
}

MessageTaskDispatcher::MessageTaskDispatcher(
    const MessageTaskDispatcher &value
)
    : controlSocket(value.controlSocket), timerSocket(value.timerSocket), taskResponse(value.taskResponse),
    deviceBestGatewayClient(value.deviceBestGatewayClient), threadUplink(value.threadUplink), parsers(value.parsers),
    regionalPlan(value.regionalPlan), identityClient(value.identityClient), queue(value.queue),
    runningUplink(value.runningUplink), onReceiveRawData(value.onReceiveRawData),
    onPushData(value.onPushData), onPullResp(value.onPullResp), onTxPkAck(value.onTxPkAck),
    onDestroy(value.onDestroy), onError(value.onError), onStart(value.onStart), onStop(value.onStop),
    onGatewayPing(value.onGatewayPing)
{
}

MessageTaskDispatcher::~MessageTaskDispatcher()
{
    stop();
    clearSockets();
    if (onDestroy)
        onDestroy(this);
}

void MessageTaskDispatcher::response(
    MessageQueueItem *item
)
{

}

void MessageTaskDispatcher::setResponse(
    TaskResponse *value
)
{
    taskResponse = value;
}

/**
 * Send buffer to client control socket
 * @param cmd buffer
 * @param size size
 */
void MessageTaskDispatcher::send2uplink(
    const void *cmd,
    size_t size
)
{
    if (controlSocket)
        write(controlSocket->sock, (const char *) cmd, (int) size);
}

/**
 * Send string to client control socket
 * @param cmd String size must be less 4K
 */
void MessageTaskDispatcher::send2uplink(
    const std::string &cmd
)
{
    send2uplink(cmd.c_str(), cmd.size());
}

void MessageTaskDispatcher::send2uplink(
    char cmd
)
{
    send2uplink(&cmd, 1);
}

/**
 * Run dispatcher uplink runner in separate thread.
 * If dispatcher running already, return true.
 * @return true always
 */
void MessageTaskDispatcher::startUplink()
{
    if (!runningUplink) {
        runningUplink = true; // force runningUplink because thread start with delay
        threadUplink = new std::thread(std::bind(&MessageTaskDispatcher::runUplink, this));
        threadUplink->detach();
    }
}

void MessageTaskDispatcher::stopUplink()
{
    if (!runningUplink)
        return;
    // wake-up select()
    char q = 'q';
    send2uplink(&q, 1);

    // wait until thread finish
    std::mutex m;
    std::unique_lock<std::mutex> lock(m);
    if (threadUplink) {
        while (runningUplink) {
            // try wake-up select() if UDP packet is missed
            send2uplink(&q, 1);
        }
        // free up resources
        delete threadUplink;
        threadUplink = nullptr;
    }
}

/**
 * Run dispatcher runner in separate threads.
 * If dispatcher running already, return true.
 * @return true always
 */
void MessageTaskDispatcher::start()
{
    startUplink();
}

/**
 * Stop dispatcher's threads.
 * Set stop request flag and send wake-up message.
 * Then wait until dispatcher loop finished then destroy dispatcher thread.
 */
void MessageTaskDispatcher::stop()
{
    stopUplink();
}

bool MessageTaskDispatcher::openSockets()
{
    bool r = true;
    for (auto s : sockets) {
        if (s->lastError || s->openSocket() == INVALID_SOCKET) {
            r = false;
            break;
        }
    }
    if (timerSocket)
        timerSocket->openSocket();
    return r;
}

void MessageTaskDispatcher::closeSockets()
{
    // close all sockets
    for (auto s : sockets) {
        s->closeSocket();
    }
    if (timerSocket) {
        timerSocket->closeSocket();
        timerSocket = nullptr;
    }
}

void MessageTaskDispatcher::clearSockets()
{
    for (auto s : sockets)
        delete s;
    // clear container
    sockets.clear();
}


SOCKET MessageTaskDispatcher::getMaxDescriptor1(
    fd_set &retValReadSet
)
{
    FD_ZERO(&retValReadSet);
    // sort sockets ascendant
    std::sort(sockets.begin(), sockets.end(),
        [] (TaskSocket* a, TaskSocket* b) {
            return a->sock < b->sock;
        }
    );
    SOCKET maxFD1 = sockets.back()->sock + 1;
    for (auto s : sockets) {
        FD_SET(s->sock, &retValReadSet);
    }
    return maxFD1;
}

/**
 * Dispatcher main uplink loop
 * @return 0 if success or negative error code
 */
int MessageTaskDispatcher::runUplink()
{
    runningUplink = true;
    if (sockets.empty()) {
        runningUplink = false;
        return ERR_CODE_PARAM_INVALID;
    }
    if (!openSockets()) {
        closeSockets();
        runningUplink = false;
        return ERR_CODE_SOCKET_CREATE;
    }

    initBridges();

    struct timeval timeout {};

    // Initialize the master fd_set
    fd_set masterReadSocketSet;
    SOCKET maxFD1 = getMaxDescriptor1(masterReadSocketSet);
    char buffer[4096];

    if (onStart) {
        onStart(this);
    }
    ParseResult pr;
    struct sockaddr srcAddr {};
    socklen_t srcAddrLen = sizeof(srcAddr);
    while (runningUplink) {
        fd_set workingSocketSet;
        // Copy the master fd_set over to the working fd_set
        memcpy(&workingSocketSet, &masterReadSocketSet, sizeof(masterReadSocketSet));
        // Initialize the timeval struct
        timeout.tv_sec = DEF_TIMEOUT_SECONDS;
        timeout.tv_usec = 0;
        int rc = select((int) maxFD1, &workingSocketSet, nullptr, nullptr, &timeout);
        if (rc < 0)     // select error
            break;

        // getUplink timestamp
        TASK_TIME receivedTime = std::chrono::system_clock::now();

        if (rc == 0) {   // select() timed out.
            if (!parsers.empty())
                sendDownlinkMessages(parsers[0]);
            cleanupOldMessages(receivedTime);
            continue;
        }

        std::vector<SOCKET> acceptedSockets;
        std::vector<TaskSocket*> removedSockets;

        // read socket(s)
        for (auto s : sockets) {
            if (!FD_ISSET(s->sock, &workingSocketSet))
                continue;
            ssize_t sz;
            switch (s->socketAccept) {
                case SA_ACCEPT_REQUIRE: {
                    // TCP, UNIX sockets require create accept socket and create a new paired socket
                    SOCKET cfd = accept(s->sock, (struct sockaddr *) &srcAddr, &srcAddrLen);
                    if (cfd > 0)
                        acceptedSockets.push_back(cfd); // do not modify vector using iterator, do it after
                    continue;
                }
                case SA_TIMER:
                    std::cout << "Timer event " << taskTime2string(receivedTime) << std::endl;
                    // read timer counter value
                    sz = read(s->sock, buffer, sizeof(buffer)); // 8 bytes, timer counter value
                    // send packets
                    sendQueue(receivedTime, pr.token);
                    // set timer
                    if (isTimeProcessQueueOrSetTimer(receivedTime)) {
                        // nothing to do
                    }
                    cleanupOldMessages(receivedTime);
                    continue;
                case SA_EVENTFD:
                    // not used
                    sz = read(s->sock, buffer, sizeof(buffer));
                    break;
                default:
                    sz = recvfrom(s->sock, buffer, sizeof(buffer), 0, &srcAddr, &srcAddrLen);
            }
            if (sz < 0) {
                std::cerr << ERR_MESSAGE  << errno << ": " << strerror(errno)
                      << " socket " << s->sock
                      << std::endl;
                if (s->socketAccept == SA_ACCEPTED) {
                    // close client connection
                    removedSockets.push_back(s); // do not modify vector using iterator, do it after
                }
                continue;
            }
            if (sz > 0) {
                switch (sz) {
                    case 1: {
                        char *a = (char *) buffer;
                        switch (*a) {
                            case 'q':
                                runningUplink = false;
                                break;
                            default:
                                break;
                        }
                    }
                        break;
                    case SIZE_DEVADDR:      // something happens on device (by address)
                    {
                        auto *a = (DEVADDR *) buffer;
                        // process message queue
                        MessageQueueItem *item = queue.findUplink(a);
                        if (item) {
                            sendPayloadOverBridge(item);
                            if (onPushData)
                                onPushData(this, item);
                        }
                        break;
                    }
                    default: {
                        if (onReceiveRawData)
                            if (!onReceiveRawData(this, buffer, sz, receivedTime))  // filter raw messages
                                continue;
                        for (auto parser: parsers) {
                            int r = parser->parse(pr, buffer, sz, receivedTime);
                            if (r == CODE_OK) {
                                r = validateGatewayAddress(pr, s, srcAddr);
                                if (r == CODE_OK) {
                                    switch (pr.tag) {
                                        case SEMTECH_GW_PUSH_DATA:
                                            pushData(s, srcAddr, pr.gwPushData, receivedTime, parser);
                                            break;
                                        case SEMTECH_GW_PULL_DATA:
                                            std::cerr << "SEND TO END-DEVICE TODO" << std::endl;
                                            break;
                                        case SEMTECH_GW_PULL_RESP:
                                            if (onPullResp)
                                                onPullResp(this, pr.gwPullResp);
                                            break;
                                        case SEMTECH_GW_TX_ACK:
                                            onTxPkAck(this, pr.code);
                                            break;
                                        default:
                                            break;
                                    }
                                }
                                // send ACK ASAP
                                if (sendACK(s, srcAddr, srcAddrLen, buffer, sz, parser) > 0) {
                                    // inform
                                }
                                break;  // otherwise try next parser
                            }
                        }
                    }
                }
            }
        }

        // if (isTimeProcessQueueOrSetTimer(receivedTime))
        //      sendQueue(receivedTime, pr.token);

        // accept connections
        if (!acceptedSockets.empty()) {
            for (auto & acceptedSocket : acceptedSockets) {
                sockets.push_back(new TaskAcceptedSocket(acceptedSocket));
            }
            acceptedSockets.clear();
            maxFD1 = getMaxDescriptor1(masterReadSocketSet);
        }
        // delete broken connections
        if (!removedSockets.empty()) {
            for (auto & removedSocket : removedSockets) {
                // getUplink max socket
                auto f = std::find_if(sockets.begin(), sockets.end(), [&removedSocket](const TaskSocket *v) {
                    return (removedSocket == v);
                });
                if (f != sockets.end())
                    sockets.erase(f);
            }
            removedSockets.clear();
            maxFD1 = getMaxDescriptor1(masterReadSocketSet);
        }
    }
    closeSockets();
    doneBridges();
    if (onStop) {
        onStop(this);
    }

    runningUplink = false;
    return CODE_OK;

}

ssize_t MessageTaskDispatcher::sendACK(
    const TaskSocket *taskSocket,
    const sockaddr &destAddr,
    socklen_t destAddrLen,
    const char *packet,
    ssize_t packetSize,
    ProtoGwParser *parser
) {
    char ack[MAX_ACK_SIZE];
    if (!parser)
        return ERR_CODE_SEND_ACK;
    ssize_t sz = parser->ack(ack, MAX_ACK_SIZE, packet, packetSize);
    if (sz <= 0)
        return sz;
    return sendto(taskSocket->sock, (const char *)  &ack, (int) sz, 0, &destAddr, (int) destAddrLen);
}

ssize_t MessageTaskDispatcher::sendConfirm(
    const TaskSocket *taskSocket,
    const sockaddr &destAddr,
    socklen_t destAddrLen
) {
    SEMTECH_ACK ack;
    return sendto(taskSocket->sock, (const char *)  &ack, SIZE_SEMTECH_ACK, 0, &destAddr, destAddrLen);
}

void MessageTaskDispatcher::setControlSocket(
    TaskSocket *socket
)
{
    auto f = std::find_if(sockets.begin(), sockets.end(), [socket](const TaskSocket *v) {
        return (socket == v);
    });
    if (f == sockets.end())
        sockets.push_back(socket);
    controlSocket = socket;
}

void MessageTaskDispatcher::pushData(
    const TaskSocket *taskSocket,
    const sockaddr &addr,
    GwPushData &pushData,
    const TASK_TIME &receivedTime,
    ProtoGwParser *parser
) {
    queueMutex.lock();
    bool isNew = queue.putUplink(receivedTime, taskSocket, addr, pushData, parser);
    queueMutex.unlock();

    if (isNew) {
    }

    auto a = pushData.rxData.getAddr();
    if (a)
        send2uplink(a, SIZE_DEVADDR);  // pass address of just added item
    if (pushData.needConfirmation())
        prepareSendConfirmation(a, addr, receivedTime);
}

void MessageTaskDispatcher::addParser(
    ProtoGwParser *aParser)
{
    parsers.push_back(aParser);
}

void MessageTaskDispatcher::setRegionalParameterChannelPlan(
    const RegionalParameterChannelPlan *aRegionalPlan
)
{
    regionalPlan = aRegionalPlan;
}

/**
 *
 * @param now
 * @param token random token
 */
void MessageTaskDispatcher::sendQueue(
    TASK_TIME now,
    uint16_t token
) {
    TimeAddr ta;
    while (queue.time2ResponseAddr.pop(ta, now)) {
        std::cout << "Pop and send from queue " << ta.toString() << "\n";
        auto m = queue.uplinkMessages.find(ta.addr);
        if (m == queue.uplinkMessages.end())
            continue;
        if (m->second.needConfirmation()) {
            ConfirmationMessage confirmationMessage(m->second.radioPacket, m->second.task);
            GatewayMetadata gwMetadata;
            // determine best gateway
            m->second.task.gatewayId = m->second.getBestGatewayAddress(gwMetadata);
            if (m->second.task.gatewayId.gatewayId) {
                // update best gateway in the storage
                if (deviceBestGatewayClient) {
                    if (deviceBestGatewayClient->svc) {
                        deviceBestGatewayClient->svc->put(ta.addr, m->second.task.gatewayId.gatewayId);
                    }
                }
                char sb[512];
                if (gwMetadata.parser) {
                    auto sz = gwMetadata.parser->makePull(sb, sizeof(sb), confirmationMessage,
                                                          token, &gwMetadata.rx, regionalPlan);
                    std::cout << "Send " << std::string(sb, sz)
                              << " to gateway " << gatewayId2str(gwMetadata.rx.gatewayId)
                              << " over socket " << gwMetadata.taskSocket->toString()
                              << std::endl;
                }
            }
        }
    }
}

bool MessageTaskDispatcher::isTimeProcessQueueOrSetTimer(
    TASK_TIME now
)
{
    // 1. check waiting time is over?
    auto d = queue.time2ResponseAddr.waitTimeForAllGatewaysInMicroseconds(now);
    if (d < 0)
        return false;

    // if time period is too small, process queue immediately
    if (d < MIN_TIMER_IN_MICROSECONDS)
        return true;

    // enough time to wait one more
    now += std::chrono::microseconds(d);

    if (!timerSocket->setStartupTime(now)) {
        if (onError)
            onError(this, LOG_CRIT, "Set timer", errno, strerror(errno));
        return true;
    }
    return false;
}

/**
 * Put address to the queue and set timer to start send confirmation after 0.5s after first message
 * @param addr address
 * @param receivedTime receive time
 */
void MessageTaskDispatcher::prepareSendConfirmation(
    const DEVADDR *addr,
    const sockaddr &sockAddr,
    TASK_TIME receivedTime
)
{
    queue.time2ResponseAddr.push(addr, receivedTime);
}

void MessageTaskDispatcher::sendDownlinkMessages(
    ProtoGwParser *proto
)
{
    for (auto it = queue.downlinkMessages.begin(); it != queue.downlinkMessages.end();) {
        const DEVADDR &a = it->first;
        GatewayMetadata gwm;
        uint64_t gw = it->second.getBestGatewayAddress(gwm);

        char buf[4096];
        uint16_t token = 1;
        SEMTECH_PROTOCOL_METADATA_RX *rxm = nullptr;

        if (gw) {
            std::cout << "send downlink to device " << DEVADDR2string(a)
                << " over gateway " << gatewayId2str(gw)
                << ' ' << sockaddr2string(&gwm.addr)
                << std::endl;
            // sendto(taskSocket->sock, (const char *) &ack, (int) sz, 0, &destAddr, (int) destAddrLen);
            MessageBuilder msgBuilder(it->second.task, it->second.radioPacket);
            ssize_t sz = proto->makePull(buf, sizeof(buf), msgBuilder, token, nullptr, regionalPlan);
            if (sz > 0)
                sendto(gwm.taskSocket->sock, (const char *) &buf, (int) sz, 0, &gwm.addr, addressLength(&gwm.addr));
        } else {
            std::cerr << "no gateway available, send over all known gateways" << std::endl;
            std::vector<GatewayIdentity> gwLs;
            identityClient->svcGateway->list(gwLs, 0, 10);
            for (auto &g : gwLs) {
                std::cout << "Send downlink to device " << DEVADDR2string(a)
                    << " over gateway " << gatewayId2str(g.gatewayId)
                    << ' ' << sockaddr2string(&g.sockaddr)
                    << " = " << it->second.toJsonString()
                    << std::endl;

                MessageBuilder msgBuilder(it->second.task, it->second.radioPacket);
                ssize_t sz = proto->makePull(buf, sizeof(buf), msgBuilder, token, nullptr, regionalPlan);
                if (sz > 0)
                    sendto(controlSocket->sock, (const char *) &buf, (int) sz, 0, &g.sockaddr, addressLength(&g.sockaddr));
            }
        }
        it = queue.downlinkMessages.erase(it);
    }
}

void MessageTaskDispatcher::cleanupOldMessages(
    TASK_TIME now
)
{
    queue.clearOldUplinkMessages(now - std::chrono::seconds(1));
}

int MessageTaskDispatcher::validateGatewayAddress(
    const ParseResult &parsedMsg,
    const TaskSocket *taskSocket,
    const sockaddr &srcSockAddr
)
{
    return CODE_OK;
}

void MessageTaskDispatcher::setIdentityClient(
    DirectClient *aIdentityClient
) {
    identityClient = aIdentityClient;
}

void MessageTaskDispatcher::addAppBridge(
    AppBridge *appBridge
)
{
    appBridge->setDispatcher(this);
    appBridges.push_back(appBridge);
}

void MessageTaskDispatcher::sendPayloadOverBridge(
    MessageQueueItem *item
)
{
    bool micMatched = item->radioPacket.matchMic(item->task.deviceId.nwkSKey);
    bool decoded = item->radioPacket.decode(&item->task.deviceId);
    for (auto b: appBridges) {
        std::cout << "Send payload, bridge " << b->name() << std::endl;
        b->onPayload(this, item, decoded, micMatched);
    }
}

void MessageTaskDispatcher::setDeviceBestGatewayClient(
    DeviceBestGatewayDirectClient *aClient
)
{
    deviceBestGatewayClient = aClient;
}

size_t MessageTaskDispatcher::bridgeCount() const
{
    return appBridges.size();
}

int MessageTaskDispatcher::sendDownlink(
    const TASK_TIME &tim,
    const DEVADDR &addr,
    void *payload,
    void *fOpts,
    uint8_t fPort,
    uint8_t payloadSize,
    uint8_t fOptsSize,
    ProtoGwParser *proto
)
{
    // check parameters
    if (payloadSize > 255)
        return ERR_CODE_WRONG_PARAM;
    if (fOptsSize > 15)
        return ERR_CODE_MAC_INVALID;
    if (!identityClient)
        return ERR_CODE_WRONG_PARAM;
    if (!identityClient->svcIdentity)
        return ERR_CODE_WRONG_PARAM;
    if (!deviceBestGatewayClient)
        return ERR_CODE_WRONG_PARAM;
    if (!deviceBestGatewayClient->svc)
        return ERR_CODE_WRONG_PARAM;

    MessageQueueItem *item = queue.getUplink(addr);
    TaskDescriptor td;
    if (item) {
        // found message from the device in the queue, let use identity and best gateway from the item
        // build downlink message
        td = item->task;
    } else {
        // no message in the queue found, getUplink identity and best gateway from the services
        // getUplink identity of the device
        DEVICEID did;
        int r = identityClient->svcIdentity->get(did, addr);
        if (r)
            return r;   // device not found, exit
        td.deviceId.devaddr = addr;
        td.deviceId = did;

        // determine best gateway
        uint64_t gwId = deviceBestGatewayClient->svc->get(addr);
        if (gwId == 0) {
            // if there are no information which gateway is the best yet, get any gateway
            // check gateway list
            if (!identityClient->svcGateway)
                return ERR_CODE_WRONG_PARAM;

            std::vector<GatewayIdentity> ls;
            // getUplink any gateway from the list
            r = identityClient->svcGateway->list(ls, 0, 1);
            if (r < 0)
                return ERR_CODE_WRONG_PARAM;    // no gateway found, exit
            if (ls.empty())
                return ERR_CODE_WRONG_PARAM;
            td.gatewayId = ls[0];
        } else
            td.gatewayId = gwId;
    }
    // build downlink message
    DownlinkMessage m(td, fPort, payload, payloadSize, fOpts, fOptsSize);
    queue.putDownlink(tim, m, proto);

    std::cout << "downlink message queued" << std::endl;

    return CODE_OK;
}

void MessageTaskDispatcher::initBridges()
{
    for (auto b(appBridges.begin()); b != appBridges.end();) {
        int r = (*b)->init("", "", nullptr);
        if (r) {
            std::stringstream ss;
            ss << "Bridge " << (*b)->name() << " initialization error";
            onError(this, LOG_ERR, MODULE_NAME_GW_UPSTREAM, r, ss.str());
            b = appBridges.erase(b);
        } else
            b++;
    }
}

void MessageTaskDispatcher::doneBridges()
{
    for (auto b: appBridges) {
        b->done();
    }
}
