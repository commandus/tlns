#ifndef TLNS_PROTO_GW_PARSER_H
#define TLNS_PROTO_GW_PARSER_H

#include "lorawan/task/task-platform.h"
#include "lorawan/task/message-task-dispatcher.h"
#include "lorawan/proto/gw/gw.h"
#include "lorawan/lorawan-builder.h"
#include "lorawan/proto/gw/parse-result.h"

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
     * @return Return tag number: 0- PUSH_DATA 2- PULL_DATA 5- TX_ACK or error code (<0)
     */
    virtual int parse(
        ParseResult &result,
        const char *packetForwarderPacket,
        size_t size,
        TASK_TIME receivedTime
    ) = 0;
    /**
     * Create ACK packet
     * @param retBuf buffer
     * @param retSize buffer size
     * @param packetForwarderPacket received packet
     * @param size received packet size
     * @return size of ACK packet. 0- no ACK packet, <0 error code e.g. buffer size is too small
     */
    virtual ssize_t ack(
        char *retBuf,
        size_t retSize,
        const char *packetForwarderPacket,
        size_t size
    ) = 0;

    virtual ssize_t makeMessage2Gateway(
        char *retBuf, size_t retSize,
        MessageBuilder &msgBuilder,
        uint16_t token,
        const SEMTECH_PROTOCOL_METADATA_RX *rxMetadata,
        const RegionalParameterChannelPlan *regionalPlan
    ) = 0;

    /**
     * Return common protocol name
     * @return NULL-terminated UTF-8 string
     */
    virtual const char* name() const = 0;

    /**
     * Unique protocol number. 1- Basic communication protocol between Lora gateway and server (C)2013 Semtech-Cycleo
     * 10..19 reserved for network server facilities
     * 20.. - user defined protocols
     * @return
     */
    virtual int tag() const = 0;

    virtual ~ProtoGwParser();

    const std::string toJsonString() const;
};

#endif
