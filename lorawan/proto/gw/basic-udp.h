#ifndef PROTO_GW_BASIC_UDP_H
#define PROTO_GW_BASIC_UDP_H    1

#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-packet-storage.h"
#include "lorawan/proto/gw/proto-gw-parser.h"

class GatewayBasicUdpProtocol : public ProtoGwParser {
private:
    // upstream {"rxpk": {}}
    static bool parsePushData(
        const char *json,
        size_t size,
        OnPushDataProc cb
    );
    // downstream {"txpk": {}}
    static bool parsePullResp(
        const char *json,
        size_t size,
        OnPullRespProc cb
    );
    // downstream {"txpk_ack": {}}
    static bool parseTxAck(
        const char *json,
        size_t size,
        OnTxpkAckProc cb
    );
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
        OnPushDataProc onPushData,
        OnPullRespProc onPullResp,
        OnTxpkAckProc onTxpkAckProc
    ) override;
};

#endif
