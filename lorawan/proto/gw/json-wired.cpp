#include <iostream>
#include <sstream>
#include "nlohmann/json.hpp"

#include "lorawan/proto/gw/json-wired.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-date.h"
#include "lorawan/power-dbm.h"

static const char *GATEWAY_JSON_WIRED_PROTOCOL_NAME = "Simple simulation wired JSON protocol";

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
 * }
 * where
 *      tag: 0- PUSH_DATA 2- PULL_DATA 5- TX_ACK
 *      token uint16_t
 *      FOpts and payload (if exists) MUST be ciphered
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
    ParseResult *rslt;
public:
    int parseError;

    explicit SaxPushData(
        ParseResult *aRslt,
        TASK_TIME receivedTime
    )
        : nameIndex(0), startItem(0), rslt(aRslt), parseError(CODE_OK)
    {
        rslt->gwPushData.rxMetadata.t = std::chrono::duration_cast<std::chrono::seconds>(receivedTime.time_since_epoch()).count();
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
                rslt->tag = (uint8_t) val;
                rslt->gwPushData.rxData.mhdr.f.mtype = (uint8_t) val;
                break;
            case 1: // token
                rslt->token = (uint16_t) val;
                break;
            default:
                break;
        }
        return true;
    }

    bool number_unsigned(number_unsigned_t val) override {
        switch (nameIndex) {
            case 0: // tag
                rslt->tag = (uint8_t) val;
                rslt->gwPushData.rxData.mhdr.f.mtype = (uint8_t) val;
                break;
            case 1: // token
                rslt->token = (uint16_t) val;
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
                rslt->gwPushData.rxMetadata.gatewayId = string2gatewayId(val);
                break;
            case 3: // devaddr, hex number, device address
                    rslt->gwPushData.rxData.mhdr.f.mtype = MTYPE_UNCONFIRMED_DATA_UP;
                    string2DEVADDR(rslt->gwPushData.rxData.data.uplink.devaddr, val);
                break;
            case 4: // FOpts, hex sequence, MAC payload (optional)
            {
                std::string h(hex2string(val));
                rslt->gwPushData.rxData.setFOpts((void *) h.c_str(), h.size());
            }
                break;
            case 5: // payload, hex sequence
            {
                std::string h(hex2string(val));
                rslt->gwPushData.rxData.setPayload((void *) h.c_str(), h.size());
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

static const char* SAX_JSON_ACK_NAMES[2] = {
    "tag",	    // 0 number 1 byte long
    "token"     // 1 number 2 bytes long
};

static inline int getAckIndex(
    const char *name
)
{
    int r = 0;
    for (int i = 0; i < 7; i++) {
        if (strcmp(SAX_JSON_ACK_NAMES[i], name) == 0)
            return i;
    }
    return r;
}

class SaxAck : public nlohmann::json::json_sax_t {
private:
    int nameIndex;
    GatewayJsonWiredAck *rslt;
public:
    int parseError;

    explicit SaxAck(
        GatewayJsonWiredAck *aRslt
    )
        : nameIndex(0), rslt(aRslt), parseError(CODE_OK)
    {
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
                if (rslt)
                    rslt->tag = (uint8_t) val;
                break;
            case 1: // token
                if (rslt)
                    rslt->token = (uint16_t) val;
                break;
            default:
                break;
        }
        return true;
    }

    bool number_unsigned(number_unsigned_t val) override {
        switch (nameIndex) {
            case 0: // tag
                if (rslt)
                    rslt->tag = (uint8_t) val;
                break;
            case 1: // token
                if (rslt)
                    rslt->token = (uint16_t) val;
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
        return true;
    }

    bool start_object(std::size_t elements) override {
        return true;
    }

    bool end_object() override {
        return true;
    }

    bool start_array(std::size_t elements) override {
        return true;
    }

    bool end_array() override {
        return true;
    }

    bool key(string_t &val) override {
        nameIndex = getAckIndex(val.c_str());
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
    ParseResult *retVal,
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
    return parsePushData(&retVal, (char *) packetForwarderPacket, size, receivedTime);
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
    ParseResult pr;
    TASK_TIME receivedTime;
    parsePushData(&pr, packet, packetSize, receivedTime);
    std::stringstream ss;
    ss << "{\"tag\": " << (int) SEMTECH_GW_PUSH_ACK
        << ", \"token\": " << pr.token
        << "}";
    std::string s = ss.str();
    size_t sz = s.size();
    if (packetSize < sz)
        return ERR_CODE_SEND_ACK;
    memmove(retBuf, s.c_str(), sz);
    return (ssize_t) sz;
}

void makeMessage(
    std::ostream &strm,
    uint16_t token,
    uint64_t gatewayId,
    const DEVADDR *addr,
    const std::string &fopts,
    const std::string &payload
) {
    strm << "{\"tag\": " << (int) SEMTECH_GW_PUSH_DATA
        << ", \"token\": " << token
        << ", \"gateway\": \"" << gatewayId2str(gatewayId);
    if (addr)
        strm << "\", \"devaddr\": \"" << addr->toString();
    strm << "\", \"fopts\": \"" << hexString(fopts)
        << "\", \"payload\": \"" << hexString(payload)
        << "\"}";
}

bool GatewayJsonWiredProtocol::makeMessage2GatewayStream(
    std::ostream &ss,
    const DEVEUI &gwId,
    MessageBuilder &msgBuilder,
    uint16_t token,
    const SEMTECH_PROTOCOL_METADATA_RX *rxMetadata,
    const RegionalParameterChannelPlan *aRegionalPlan
)
{
    if (!rxMetadata)
        return false;
    makeMessage(ss, token, rxMetadata->gatewayId, msgBuilder.msg.getAddr(), msgBuilder.msg.foptsString(), msgBuilder.msg.payloadString());
    ss << "{\"tag\": " << (int) SEMTECH_GW_PUSH_DATA
        << ", \"token\": " << token
        << ", \"gateway\": \"" << gatewayId2str(rxMetadata->gatewayId)
        << "\", \"devaddr\": \"" << msgBuilder.msg.getAddr()->toString()
        << "\", \"fopts\": \"" << hexString(msgBuilder.msg.foptsString())
        << "\", \"payload\": \"" << hexString(msgBuilder.msg.payloadString())
        << "\"}";
    return true;
}

ssize_t GatewayJsonWiredProtocol::makePull(
    char *retBuf,
    size_t retSize,
    const DEVEUI &gwId,
    MessageBuilder &msgBuilder,
    uint16_t token,
    const SEMTECH_PROTOCOL_METADATA_RX *rxMetadata,
    const RegionalParameterChannelPlan *regionalPlan
)
{
    std::stringstream ss;
    if (!makeMessage2GatewayStream(ss, gwId, msgBuilder, token, rxMetadata, regionalPlan))
        return ERR_CODE_PARAM_INVALID;
    std::string s(ss.str());
    auto sz = s.size();
    if (retBuf && retSize >= sz)
        memmove(retBuf, s.c_str(), sz);
    return (ssize_t) sz;
}

const char *GatewayJsonWiredProtocol::name() const
{
    return GATEWAY_JSON_WIRED_PROTOCOL_NAME;
}

int GatewayJsonWiredProtocol::tag() const
{
    return 11;
}

GatewayJsonWiredAck::GatewayJsonWiredAck()
    : tag(SEMTECH_GW_PUSH_ACK), token(0)
{

}

int parseAck(
    GatewayJsonWiredAck *retVal,
    const char *json,
    size_t jsonSize

) {
    SaxAck consumer(retVal);
    nlohmann::json::sax_parse(json, json + jsonSize, &consumer);
    return consumer.parseError;
}
