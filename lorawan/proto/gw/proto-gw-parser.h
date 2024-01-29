#ifndef TLNS_PROTO_GW_PARSER_H
#define TLNS_PROTO_GW_PARSER_H

class GwPushData {
public:
    uint64_t gwId;
    LorawanPacketStorage rxData;
    SEMTECH_PROTOCOL_METADATA_RX rxMetadata;
};

class GwPullResp {
public:
    uint64_t gwId;
    LorawanPacketStorage txData;
    SEMTECH_PROTOCOL_METADATA_TX txMetadata;
};

typedef void(*OnPushDataProc)(
    GwPushData &item
);

typedef void(*OnPullRespProc)(
    GwPullResp &item
);

typedef void(*OnTxpkAckProc)(
    ERR_CODE_TX code
);

class ProtoGwParser {
public:
    /** Upstream only. array of packets from Basic communication protocol packet
     * @param packetForwarderPacket
     * @param size
     * @param cb put gateway identifier (if supplied, tags: 0- PUSH_DATA 2- PULL_DATA 5- TX_ACK)
     * @return Return tag number 0-5 or error code (<0)
     */
    virtual int parse(
        const char *packetForwarderPacket,
        size_t size,
        OnPushDataProc onPushData,
        OnPullRespProc onPullResp,
        OnTxpkAckProc onTxPkAckProc
    ) = 0;
};

#endif //TLNS_PROTO_GW_PARSER_H
