#include "nlohmann/json.hpp"

#include "lorawan/proto/gw/basic-udp.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-date.h"

static const char* SAX_METADATA_RX_NAMES [15] = {
    "rxpk",	// 0 array name
    "time", // 1 string | UTC time of pkt RX, us precision, ISO 8601 'compact' format
    "tmms", // 2 unsigned | GPS time of pkt RX, number of milliseconds since 06.Jan.1980
    "tmst", // 3 unsigned | Internal timestamp of "RX finished" event (32b)
    "freq", // 4 float, unsigned | RX central frequency in MHz (Hz precision)
    "chan", // 5 unsigned | Concentrator "IF" channel used for RX
    "rfch", // 6 unsigned | Concentrator \"RF chain\" used for RX
    "stat", // 7 unsigned | CRC status: 1 = OK, -1 = fail, 0 = no CRC
    "modu", // 8 string | Modulation identifier "LORA" or "FSK"
    "datr", // 9 string | LoRa datarate identifier (eg. SF12BW500)
            // 10 unsigned | FSK datarate (unsigned, in bits per second)
    "codr", // 11 string | LoRa ECC coding rate identifier
    "rssi", // 12 signed | RSSI in dBm (1 dB precision)
    "lsnr", // 13 float, signed | Lora SNR ratio in dB (signed float, 0.1 dB precision)
    "size", // 14 unsigned | RF packet payload size in bytes
    "data"  // 15 string | Base64 encoded RF packet payload, padded
};

static inline int getMetadataRxNameIndex(
    const char *name
)
{
    int r = 0;
    for (int i = 0; i < 15; i++) {
        if (strcmp(SAX_METADATA_RX_NAMES[i], name) == 0)
            return i;
    }
    return r;
}

class SaxPushData : public nlohmann::json::json_sax_t {
private:
    int nameIndex;
    int startItem;  // object enter/exit counter
    GwPushData item;
    OnPushDataProc cb;
public:
    explicit SaxPushData(OnPushDataProc aCb)
        : nameIndex(0), startItem(0), cb(aCb)
    {

    }

    bool null() override {
        return true;
    }

    bool boolean(bool val) override {
        return true;
    }

    bool number_integer(number_integer_t val) override {
        events.push_back("number_integer(val=" + std::to_string(val) + ")");
        return true;
    }

    bool number_unsigned(number_unsigned_t val) override {
        events.push_back("number_unsigned(val=" + std::to_string(val) + ")");
        return true;
    }

    bool number_float(number_float_t val, const string_t &s) override {
        events.push_back("number_float(val=" + std::to_string(val) + ", s=" + s + ")");
        return true;
    }

    bool string(string_t &val) override {
        switch (nameIndex) {
            case 1: // time
                item.rxMetadata.t = parseDate(val.c_str());
                break;
            case 8: // modu
                item.rxMetadata.modu = string2MODULATION(val.c_str());
                break;
            case 9: // datr string
                item.rxMetadata. = string2datr(val.c_str());
                break;
            case 10: // codr
                break;
            case 14: // data
                break;
        }
        return true;
    }

    bool start_object(std::size_t elements) override {
        startItem++;
        return true;
    }

    bool end_object() override {
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
        nameIndex = getMetadataRxNameIndex(val.c_str());
        return true;
    }

    bool binary(nlohmann::json::binary_t &val) override {
        return true;
    }

    bool parse_error(std::size_t position, const std::string &last_token, const json::exception &ex) override {
        return false;
    }
};

int GatewayBasicUdpProtocol::parse(
    const char *packetForwarderPacket,
    size_t size,
    OnPushDataProc onPushData,
    OnPullRespProc onPullResp,
    OnTxpkAckProc onTxpkAckProc
)
{
    if (size <= sizeof(SEMTECH_PREFIX)) // at least 4 bytes
        return ERR_CODE_INVALID_PACKET;
    SEMTECH_PREFIX *p = (SEMTECH_PREFIX *) packetForwarderPacket;
    if (p->version != 2)
        return ERR_CODE_INVALID_PACKET;
    int r = p->tag;
    if (r > 5)
        return ERR_CODE_INVALID_PACKET;

    switch (p->tag) {
        case SEMTECH_GW_PUSH_DATA:  // 0 network server responds on PUSH_DATA to acknowledge immediately all the PUSH_DATA packets received
            parsePushData((char *) packetForwarderPacket + SIZE_SEMTECH_PREFIX_GW, size - SIZE_SEMTECH_PREFIX_GW, onPushData); // +12 bytes
            break;
        case SEMTECH_GW_PULL_RESP:  // 4
            parsePullResp((char *) packetForwarderPacket + SIZE_SEMTECH_PREFIX, size - SIZE_SEMTECH_PREFIX, onPullResp); // +4 bytes
            break;
        case SEMTECH_GW_TX_ACK:     // 5 gateway inform network server about does PULL_RESP data transmission was successful or not
            parseTxAck((char *) packetForwarderPacket + SIZE_SEMTECH_PREFIX_GW, SIZE_SEMTECH_PREFIX_GW, onTxpkAckProc); // +12 bytes
            break;
        default:
    }
    return r;
}

bool GatewayBasicUdpProtocol::parsePushData(
    const char *json,
    size_t size,
    OnPushDataProc cb
) {
    SaxPushData consumer(cb);
    nlohmann::json::sax_parse(json, json + size, &consumer);
}

bool GatewayBasicUdpProtocol::parsePullResp(
    const char *json,
    size_t size,
    OnPullRespProc cb
) {

}

bool GatewayBasicUdpProtocol::parseTxAck(
    const char *json,
    size_t size,
    OnTxpkAckProc cb
) {
    ERR_CODE_TX code;

    nlohmann::json js = nlohmann::json::parse(json, json + size);
    if (!js.is_object())
        return false;
    if (!js.contains("txpk_ack"))
        return false;
    auto jAck = js["txpk_ack"];
    if (!jAck.is_object())
        return false;
    if (!js.contains("error"))
        return false;
    auto jError = js["error"];
    if (!jError.is_string())
        return false;
    code = string2ERR_CODE_TX(jError);
    if (cb)
        cb(code);
}
