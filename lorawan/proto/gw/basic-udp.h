#ifndef PROTO_GW_BASIC_UDP_H
#define PROTO_GW_BASIC_UDP_H    1

/**
 * Basic communication protocol between Lora gateway and server (C)2013 Semtech-Cycleo
 * @see https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT
 */
#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-packet-storage.h"
#include "lorawan/proto/gw/proto-gw-parser.h"

/**
 * Parse upstream PUSH message {"rxpk": {}}
 * @param retVal returned received message form the device
 * @param json JSON to parse
 * @param size size of JSON char array
 * @param gwId gateway identifier
 * @param receivedTime time of receive
 * @return 0- success
 */
int parsePushData(
    GwPushData *retVal,
    const char *json,
    size_t size,
    const DEVEUI &gwId,
    TASK_TIME receivedTime
);

/**
 * Parse message to downstream from the server to the device  {"txpk": {}}
 * @param retVal returned value
 * @param json JSON to parse
 * @param size size of JSON char array
 * @param gwId gateway identifier
 * @return 0- success
 */
int parsePullResp(
    GwPullResp *retVal,
    const char *json,
    size_t size,
    const DEVEUI &gwId
);

/**
 * Parse ACK message downstream {"txpk_ack": {}}
 * @param retVal returned code
 * @param json JSON to parse
 * @param size size of JSON char array
 * @return
 */
int parseTxAck(
    ERR_CODE_TX *retVal,
    const char *json,
    size_t size
);

/**
 * Semtech-Cycleo Basic communication protocol between Lora gateway and server
 * @see https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT#L133C1-L147C58
 */
class GatewayBasicUdpProtocol : public ProtoGwParser {
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
     * @param cb putUplink gateway identifier (if supplied, tags: 0- PUSH_DATA 2- PULL_DATA 5- TX_ACK)
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

    int tag() const override;
    const char* name() const override;

    explicit GatewayBasicUdpProtocol(MessageTaskDispatcher *dispatcher);
};

#endif
