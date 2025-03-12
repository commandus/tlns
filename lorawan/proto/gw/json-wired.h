#ifndef PROTO_GW_JSON_WIRED_H
#define PROTO_GW_JSON_WIRED_H    1

#include "lorawan/lorawan-types.h"
#include "lorawan/proto/gw/proto-gw-parser.h"

/**
 * Simple simulation wired JSON protocol
 * Example:
 * {
 *      "tag": 0,
 *      "token": 1001,
 *      "gateway": "aabb12cc34",
 *      "devaddr": "0011",
 *      "fopts": "",
 *      "payload": "ffaa11"
 * }
 * where
 *      tag: 0- PUSH_DATA 2- PULL_DATA 5- TX_ACK
 *      token uint16_t
 *      FOpts and payload (if exists) MUST be ciphered
 */
class GatewayJsonWiredProtocol : public ProtoGwParser {
protected:
    static bool makeMessage2GatewayStream(
        std::ostream &retStrm,
        const DEVEUI &gwId,
        MessageBuilder &msgBuilder,
        uint16_t token,
        const SEMTECH_PROTOCOL_METADATA_RX *rxMetadata,
        const RegionalParameterChannelPlan *aRegionalPlan
    );
public:
    /** Upstream only. array of packets from Basic communication protocol packet
     * @param packetForwarderPacket JSON text buffer
     * @param size JSON text buffer size
     * @param receivedTime time
     * @return Return tag number 0-5 or error code (<0)
     */
    int parse(
        ParseResult &retVal,
        const char *packetForwarderPacket,
        size_t size,
        TASK_TIME receivedTime
    ) override;

    ssize_t ack(
        char *retBuf,
        size_t retSize,
        const char *packetForwarderPacket,
        size_t size
    ) override;

    ssize_t makePull(
        char *retBuf,
        size_t retSize,
        const DEVEUI &gwId,
        MessageBuilder &msgBuilder,
        uint16_t token,
        const SEMTECH_PROTOCOL_METADATA_RX *rxMetadata,
        const RegionalParameterChannelPlan *pPlan
    ) override;

    int tag() const override;
    const char* name() const override;

    explicit GatewayJsonWiredProtocol(MessageTaskDispatcher *dispatcher);
};

class GatewayJsonWiredAck {
public:
    uint8_t tag;
    uint16_t token;
    GatewayJsonWiredAck();
};

void makeMessage(
    std::ostream &strm,
    uint16_t token,
    uint64_t gatewayId,
    const DEVADDR *addr,
    const std::string &fopts,
    const std::string &payload
);

int parseAck(
    GatewayJsonWiredAck *retVal,
    const char *json,
    size_t jsonSize

);

#endif
