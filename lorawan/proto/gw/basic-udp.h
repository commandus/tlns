#ifndef PROTO_GW_BASIC_UDP_H
#define PROTO_GW_BASIC_UDP_H    1

#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-packet-storage.h"

class GwRxItem {
public:
    uint64_t gwId;
    LorawanPacketStorage rxData;
    SEMTECH_PROTOCOL_METADATA rxMetadata;
};

typedef void(*PutGwRxItemProc)(
    GwRxItem &item
);

class GatewayBasicUdpProtocol {
protected:
    static int parsePrefixGw(
        SEMTECH_PREFIX_GW &retprefix,
        const void *packetForwarderPacket,
        size_t size
    );
    static char *getMetadataJSONPtr(
        const void *packet,
        size_t size
    );

public:
    /** array of packets from Basic communication protocol packet
     * @param packetForwarderPacket
     * @param size
     * @param cb put gateway identifier (if supplied, tags: 0- PUSH_DATA 2- PULL_DATA 5- TX_ACK)
     * @return Return tag number 0-5 or error code (<0)
     */
    static int parse(
        const char *packetForwarderPacket,
        size_t size,
        PutGwRxItemProc cb
    );
};

#endif