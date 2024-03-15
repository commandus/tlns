#ifndef TLNS_PROTO_GW_PARSER_H
#define TLNS_PROTO_GW_PARSER_H

#include "lorawan/task/task-platform.h"
#include "lorawan/task/message-task-dispatcher.h"
#include "lorawan/proto/gw/gw.h"

/**
 * @see GatewayBasicUdpProtocol
 * @file basic-udp.h
 */
class ProtoGwParser {
public:
    MessageTaskDispatcher *dispatcher;
    explicit ProtoGwParser(MessageTaskDispatcher *dispatcher);

    /** Upstream only. array of packets from Basic communication protocol packet
     * @param packetForwarderPacket buffer
     * @param size buffer size
     * @param receivedTime time
     * @param cb put gateway identifier (if supplied, tags: 0- PUSH_DATA 2- PULL_DATA 5- TX_ACK)
     * @param onPushData callback
     * @param onPullResp callback
     * @param onTxPkAck callback
     * @return Return tag number 0-5 or error code (<0)
     */
    virtual int parse(
        const char *packetForwarderPacket,
        size_t size,
        TASK_TIME receivedTime,
        OnPushDataProc onPushData,
        OnPullRespProc onPullResp,
        OnTxpkAckProc onTxPkAck
    ) = 0;
};

#endif
