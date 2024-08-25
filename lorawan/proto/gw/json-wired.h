#ifndef PROTO_GW_JSON_WIRED_H
#define PROTO_GW_JSON_WIRED_H    1

#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-packet-storage.h"
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
 *      "payload": "ffaa11",
 *      "region": "EU868"
 * }
 * where
 *      tag: 0- PUSH_DATA 2- PULL_DATA 5- TX_ACK
 *      token uint16_t
 *      FOpts and payload (if exists) MUST be ciphered
 *      region (if specified) is used to set gateway metadata - random frequency, RSSI etc.
 */
class GatewayJsonWiredProtocol : public ProtoGwParser {
protected:
    static bool makeMessage2GatewayStream(
        std::ostream &retStrm,
        MessageBuilder &msgBuilder,
        uint16_t token,
        const SEMTECH_PROTOCOL_METADATA_RX *rxMetadata,
        const RegionalParameterChannelPlan *aRegionalPlan
    );
public:
    /** Upstream only. array of packets from Basic communication protocol packet
     * @param packetForwarderPacket
     * @param size
     * @param cb put gateway identifier (if supplied, tags: 0- PUSH_DATA 2- PULL_DATA 5- TX_ACK)
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

    ssize_t makeMessage2Gateway(
        char *retBuf,
        size_t retSize,
        MessageBuilder &msgBuilder,
        uint16_t token,
        const SEMTECH_PROTOCOL_METADATA_RX *rxMetadata,
        const RegionalParameterChannelPlan *pPlan
    ) override;

    explicit GatewayJsonWiredProtocol(MessageTaskDispatcher *dispatcher);
};

#endif
