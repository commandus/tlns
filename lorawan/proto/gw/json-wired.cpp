#include <iostream>
#include <sstream>
#include "nlohmann/json.hpp"

#include "lorawan/proto/gw/json-wired.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-date.h"
#include "lorawan/power-dbm.h"

/*
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

/**
 * 	Section 3.3
 */
static const char* SAX_JSON_WIRED_NAMES[6] = {
    "tag",	    // 0 number 1 byte long
    "token",    // 1 number 2 bytes long
    "gateway",  // 2 string hex number, gateway identifier
    "devaddr",  // 3 string hex number, device address
    "fopts",    // 4 string hex sequence, FOpts payload (optional)
    "payload"  // 5 string hex sequence, payload (optional)
};

static inline int getNameIndex(
    const char *name
)
{
    int r = 0;
    for (int i = 0; i < 7; i++) {
        if (strcmp(SAX_JSON_WIRED_NAMES[i], name) == 0)
            return i;
    }
    return r;
}

class SaxPushData : public nlohmann::json::json_sax_t {
private:
    int nameIndex;
    int startItem;  // object enter/exit counter
    GwPushData *item;
public:
    int parseError;

    explicit SaxPushData(
        GwPushData *retVal,
        TASK_TIME receivedTime
    )
        : nameIndex(0), startItem(0), item(retVal), parseError(CODE_OK)
    {
        item->rxMetadata.t = std::chrono::duration_cast<std::chrono::seconds>(receivedTime.time_since_epoch()).count();
    }

    bool null() override {
        return true;
    }

    bool boolean(bool val) override {
        return true;
    }

    bool number_integer(number_integer_t val) override {
        switch (nameIndex) {
            case 0: // tag
                item->rxData.mhdr.f.mtype = (int8_t) val;
                break;
            default:
                break;
        }
        return true;
    }

    bool number_unsigned(number_unsigned_t val) override {
        switch (nameIndex) {
            case 0: // tag
                item->rxData.mhdr.f.mtype = (int8_t) val;
            break;
            default:
                break;
        }
        return true;
    }

    bool number_float(number_float_t val, const string_t &s) override {
        return true;
    }

    bool string(string_t &val) override {
        switch (nameIndex) {
            case 2: // gateway, hex number, gateway identifier
                item->rxMetadata.gatewayId = string2gatewayId(val);
                break;
            case 3: // devaddr, hex number, device address
                    item->rxData.mhdr.f.mtype = MTYPE_UNCONFIRMED_DATA_UP;
                    string2DEVADDR(item->rxData.data.uplink.devaddr, val);
                break;
            case 4: // FOpts, hex sequence, MAC payload (optional)
            {
                std::string h(hex2string(val));
                item->rxData.setFOpts((void *) h.c_str(), h.size());
            }
                break;
            case 5: // payload, hex sequence
            {
                std::string h(hex2string(val));
                item->rxData.setPayload((void *) h.c_str(), h.size());
            }
                break;
            default:
                break;
        }
        return true;
    }

    bool start_object(std::size_t elements) override {
        startItem++;
        return true;
    }

    bool end_object() override {
        if (startItem == 2) {// time to add next packet
            // ret
        }
        startItem--;
        return true;
    }

    bool start_array(std::size_t elements) override {
        return true;
    }

    bool end_array() override {
        return true;
    }

    bool key(string_t &val) override {
        nameIndex = getNameIndex(val.c_str());
        return true;
    }

    bool binary(nlohmann::json::binary_t &val) override {
        return true;
    }

    bool parse_error(std::size_t position, const std::string &last_token, const nlohmann::json::exception &ex) override {
        parseError = - ex.id;
        std::cerr << ex.what() << std::endl;
        return false;
    }
};

// ---------------------------------------------------------------------------------------------------------------------

static int parsePushData(
    GwPushData *retVal,
    const char *json,
    size_t size,
    TASK_TIME receivedTime
) {
    SaxPushData consumer(retVal, receivedTime);
    nlohmann::json::sax_parse(json, json + size, &consumer);
    return consumer.parseError;
}

int GatewayJsonWiredProtocol::parse(
    ParseResult &retVal,
    const char *packetForwarderPacket,
    size_t size,
    TASK_TIME receivedTime
)
{
    if (size <= sizeof(SEMTECH_PREFIX)) // at least 4 bytes
        return ERR_CODE_INVALID_PACKET;
    auto *p = (SEMTECH_PREFIX *) packetForwarderPacket;
    if (p->version != 2)
        return ERR_CODE_INVALID_PACKET;
    retVal.tag = p->tag;
    retVal.token = p->token;
    int r;
    switch (p->tag) {
        case SEMTECH_GW_PUSH_DATA:  // 0 network server responds on PUSH_DATA to acknowledge immediately all the PUSH_DATA packets received
            r = parsePushData(&retVal.gwPushData, (char *) packetForwarderPacket, size, receivedTime);
            break;
        case SEMTECH_GW_PULL_RESP:  // 4
            r = 0;
            break;
        case SEMTECH_GW_TX_ACK:     // 5 gateway inform network server about does PULL_RESP data transmission was successful or not
            r = 0;
            break;
        default:
            r = ERR_CODE_INVALID_PACKET;
    }
    return r;
}

GatewayJsonWiredProtocol::GatewayJsonWiredProtocol(
    MessageTaskDispatcher *aDispatcher
)
    : ProtoGwParser(aDispatcher)
{

}

ssize_t GatewayJsonWiredProtocol::ack(
    char *retBuf,
    size_t retSize,
    const char *packet,
    size_t packetSize
)
{
    return 0;
}

bool GatewayJsonWiredProtocol::makeMessage2GatewayStream(
    std::ostream &ss,
    MessageBuilder &msgBuilder,
    uint16_t token,
    const SEMTECH_PROTOCOL_METADATA_RX *rxMetadata,
    const RegionalParameterChannelPlan *aRegionalPlan
)
{
    if (!rxMetadata)
        return false;
    ss << "{}";
    return true;
}

ssize_t GatewayJsonWiredProtocol::makeMessage2Gateway(
    char *retBuf,
    size_t retSize,
    MessageBuilder &msgBuilder,
    uint16_t token,
    const SEMTECH_PROTOCOL_METADATA_RX *rxMetadata,
    const RegionalParameterChannelPlan *regionalPlan
)
{
    std::stringstream ss;
    if (!makeMessage2GatewayStream(ss, msgBuilder, token, rxMetadata, regionalPlan))
        return ERR_CODE_PARAM_INVALID;
    std::string s(ss.str());
    auto sz = s.size();
    if (retBuf && retSize >= sz)
        memmove(retBuf, s.c_str(), sz);
    return (ssize_t) sz;
}
