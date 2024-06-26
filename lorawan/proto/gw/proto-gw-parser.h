#ifndef TLNS_PROTO_GW_PARSER_H
#define TLNS_PROTO_GW_PARSER_H

#include "lorawan/task/task-platform.h"
#include "lorawan/task/message-task-dispatcher.h"
#include "lorawan/proto/gw/gw.h"

/**
 * ProtoGwParser::parse return result in ParseResult structure
 */
class ParseResult {
public:
    uint8_t tag;
    GwPushData gwPushData;
    GwPullResp gwPullResp;
    ERR_CODE_TX code;           ///< code
};

/**
  * MessageTaskDispatcher call ProtoGwParser::parse() method
  * to parse received message from the TaskSocket.
  * You must override this abstract class to implement specific protocol
  * For instance GatewayBasicUdpProtocol is implementation of Semtech basic gateway protocol.
  * @see GatewayBasicUdpProtocol
  * @file basic-udp.h
  */
class ProtoGwParser {
public:
    MessageTaskDispatcher *dispatcher;
    explicit ProtoGwParser(MessageTaskDispatcher *dispatcher);

    /**
     * Parse protocol interface
     * Upstream only. array of packets from Basic communication protocol packet
     * @param result return value
     * @param packetForwarderPacket buffer
     * @param size buffer size
     * @param receivedTime time
     * @param cb put gateway identifier (if supplied, tags: 0- PUSH_DATA 2- PULL_DATA 5- TX_ACK)
     * @return Return tag number 0-5 or error code (<0)
     */
    virtual int parse(
        ParseResult &result,
        const char *packetForwarderPacket,
        size_t size,
        TASK_TIME receivedTime
    ) = 0;
    virtual ~ProtoGwParser();
};

#endif
