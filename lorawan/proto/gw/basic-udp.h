#ifndef PROTO_GW_BASIC_UDP_H
#define PROTO_GW_BASIC_UDP_H    1

#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-packet-storage.h"
#include "lorawan/proto/gw/proto-gw-parser.h"

// upstream {"rxpk": {}}
int parsePushData(
    GwPushData *retVal,
    const char *json,
    size_t size,
    const DEVEUI &gwId,
    TASK_TIME receivedTime
);

// downstream {"txpk": {}}
int parsePullResp(
    GwPullResp *retVal,
    const char *json,
    size_t size,
    const DEVEUI &gwId
);

// downstream {"txpk_ack": {}}
int parseTxAck(
    ERR_CODE_TX *retVal,
    const char *json,
    size_t size
);

class GatewayBasicUdpProtocol : public ProtoGwParser {
public:
    /** Upstream only. array of packets from Basic communication protocol packet
     * @param packetForwarderPacket
     * @param size
     * @param cb put gateway identifier (if supplied, tags: 0- PUSH_DATA 2- PULL_DATA 5- TX_ACK)
     * @return Return tag number 0-5 or error code (<0)
     */
    int parse(
        const char *packetForwarderPacket,
        size_t size,
        TASK_TIME receivedTime,
        OnPushDataProc onPushData,
        OnPullRespProc onPullResp,
        OnTxpkAckProc onTxpkAckProc
    ) override;

    GatewayBasicUdpProtocol(MessageTaskDispatcher *pDispatcher);

};

#endif
