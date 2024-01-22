#include <cstring>

#include "lorawan/proto/gw/basic-udp.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-conv.h"

char *GatewayBasicUdpProtocol::getMetadataJSONPtr(
    const void *packet,
    size_t size
)
{
    if (size <= sizeof(SEMTECH_PREFIX)) // at least 4 bytes
        return nullptr;
    switch (((SEMTECH_PREFIX *) packet)->tag) {
        case SEMTECH_GW_PUSH_DATA:  // 0 network server responds on PUSH_DATA to acknowledge immediately all the PUSH_DATA packets received
            return (char *) packet + SIZE_SEMTECH_PREFIX_GW; // 12 bytes
        case SEMTECH_GW_PUSH_ACK:   // 1 gateway initiate receiving packets from the network server (because of NAT)
            return nullptr;         // does not contain any data. Prefix has SIZE_SEMTECH_PREFIX = 4 bytes
        case SEMTECH_GW_PULL_DATA:  // 2 network server responds on PULL_DATA
            return nullptr;         // does not contain any data. Prefix has SIZE_SEMTECH_PREFIX_GW = 12 bytes
        case SEMTECH_GW_PULL_ACK:   // 3 network server send packet to the gateway after PULL_DATA - PULL_ACK sequence
            return nullptr;         // does not contain any data. Prefix has SIZE_SEMTECH_PREFIX = 4 bytes
        case SEMTECH_GW_PULL_RESP:  // 4
            return (char *) packet + SIZE_SEMTECH_PREFIX; // 4 bytes
        case SEMTECH_GW_TX_ACK:     // 5 gateway inform network server about does PULL_RESP data transmission was successful or not
            return (char *) packet + SIZE_SEMTECH_PREFIX_GW; // 12 bytes
        default:
            return nullptr;
    }
}

/**
 * Parse Semtech UDP packet gateway prefix
 * @return 0, ERR_CODE_PACKET_TOO_SHORT, ERR_CODE_INVALID_PROTOCOL_VERSION
 */
int GatewayBasicUdpProtocol::parsePrefixGw(
    SEMTECH_PREFIX_GW &retprefix,
    const void *packetForwarderPacket,
    size_t size
)
{
    if (size < sizeof(SEMTECH_PREFIX_GW))
        return ERR_CODE_PACKET_TOO_SHORT;
    memmove(&retprefix, packetForwarderPacket, sizeof(SEMTECH_PREFIX_GW));
    *(uint64_t *) &(retprefix.mac) = NTOH8(*(uint64_t *) &(retprefix.mac));
    // check version
    if (retprefix.version != 2)
        return ERR_CODE_INVALID_PROTOCOL_VERSION;
    return CODE_OK;
}

int GatewayBasicUdpProtocol::parse(
    const char *packetForwarderPacket,
    size_t size,
    PutGwRxItemProc cb
)
{

}
