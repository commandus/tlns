#include <iostream>
#include <sstream>
#include "nlohmann/json.hpp"

#include "lorawan/proto/gw/basic-udp.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-date.h"
#include "lorawan/power-dbm.h"

static const char *GATEWAY_BASIC_UDP_PROTOCOL_NAME = "Basic communication protocol between Lora gateway and server";
/**
 * 	Section 3.3
 */
static const char* SAX_METADATA_RX_NAMES [15] = {
    "rxpk",	// 0 array name
    "time", // 1 string | UTC time of pkt RX, us precision, ISO 8601 'compact' format
    "tmms", // 2 unsigned | GPS time of pkt RX, number of milliseconds since 06.Jan.1980
    "tmst", // 3 unsigned | Internal timestamp of "RX finished" event (32b)
    "freq", // 4 float, unsigned | RX central frequency in MHz (Hz precision)
    "chan", // 5 unsigned | Concentrator "IF" channel used for RX
    "rfch", // 6 unsigned | Concentrator \"RF chain\" used for RX
    "stat", // 7 unsigned | CRC status: 1 = OK, -1 = fail, 0 = no CRC
    "modu", // 8 string | Modulation identifier "MODULATION_LORA" or "MODULATION_FSK"
    "datr", // 9 string | LoRa datarate identifier (eg. SF12BW500)
            // unsigned | MODULATION_FSK datarate (unsigned, in bits per second)
    "codr", // 10 string | LoRa ECC coding rate identifier
    "rssi", // 11 signed | RSSI in dBm (1 dB precision)
    "lsnr", // 12 float, signed | Lora SNR ratio in dB (signed float, 0.1 dB precision)
    "size", // 13 unsigned | RF packet payload size in bytes
    "data"  // 14 string | Base64 encoded RF packet payload, padded
};

static const char* SAX_METADATA_TXPK_ACK_NAMES [2] = {
    "txpk_ack",
    "error"
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
    GwPushData *item;
public:
    int parseError;

    explicit SaxPushData(
        GwPushData *retVal,
        const DEVEUI &gwId,
        TASK_TIME receivedTime
    )
        : nameIndex(0), startItem(0), item(retVal), parseError(CODE_OK)
    {
        item->rxMetadata.gatewayId = gwId.u;
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
            case 11: // rssi
                item->rxMetadata.rssi = (int16_t ) val;
                break;
            case 12: // lsnr
                item->rxMetadata.lsnr = (float) val;
                break;
            default:
                break;
        }
        return true;
    }

    bool number_unsigned(number_unsigned_t val) override {
        switch (nameIndex) {
            case 2: // tmms
                item->rxMetadata.tmst = gps2utc(val);
                break;
            case 3: // tmst
                item->rxMetadata.tmst = val;
                break;
            case 4: // freq, uint
                item->rxMetadata.freq = val * 1000000;
                break;
            case 5: // chan
                item->rxMetadata.chan = (uint8_t) val;
                break;
            case 6: // rfch
                item->rxMetadata.rfch = val;
                break;
            case 7: // stat
                item->rxMetadata.stat = (int8_t) val;
                break;
            case 9: // datr, MODULATION_FSK bits per second
                item->rxMetadata.bps = val;
                break;
            case 11: // rssi
                item->rxMetadata.rssi = (int16_t) val;
                break;
            case 12: // lsnr
                item->rxMetadata.lsnr = (float) val;
                break;
            // case 13: // size
            //    break;
            default:
                break;
        }
        return true;
    }

    bool number_float(number_float_t val, const string_t &s) override {
        switch (nameIndex) {
            case 4: // freq
                item->rxMetadata.freq = (uint16_t) (val * 1000000);
                break;
            case 12: // lsnr
                item->rxMetadata.lsnr = (float) val;
                break;
            default:
                break;
        }
        return true;
    }

    bool string(string_t &val) override {
        switch (nameIndex) {
            case 1: // time
                item->rxMetadata.t = parseDate(val.c_str());
                break;
            case 8: // modu
                item->rxMetadata.modu = string2MODULATION(val.c_str());
                break;
            case 9: // datr string
            {
                BANDWIDTH b;
                item->rxMetadata.spreadingFactor = string2datr(b, val);
                item->rxMetadata.bandwidth = b;
            }
                break;
            case 10: // codr
                item->rxMetadata.codingRate = string2codingRate(val);
                break;
            case 14: // data
                base64SetToLORAWAN_MESSAGE_STORAGE(item->rxData, val);
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
        nameIndex = getMetadataRxNameIndex(val.c_str());
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

/**
 * 	Section 6
 */
static const char* SAX_METADATA_TX_NAMES [16] = {
    "txpk",	// 0 array name
    "imme", // 1 bool            Send packet immediately (will ignore tmst & time)
    "tmst", // 2 unsigned        Send packet on a certain timestamp value (will ignore time)
    "tmms", // 3 unsigned        Send packet at a certain GPS time (GPS synchronization required)
    "freq", // 4 float, unsigned TX central frequency in MHz (Hz precision)
    "rfch", // 5 unsigned        Concentrator "RF chain" used for TX
    "powe", // 6 unsigned        TX output power in dBm (dBm precision)
    "modu", // 7 string          Modulation identifier "MODULATION_LORA" or "MODULATION_FSK"
    "datr", // 8 string          LoRa data rate identifier e.g. SF12BW500
            //   unsigned        or MODULATION_FSK data rate in bits per second
    "codr", // 9 string          LoRa ECC coding rate identifier
    "fdev", // 10 unsigned       MODULATION_FSK frequency deviation in Hz
    "ipol", // 11 bool           Lora modulation polarization inversion
    "prea", // 12 unsigned       RF preamble size
    "size", // 13 unsigned       RF packet payload size in bytes
    "data", // 14 string         Base64 encoded RF packet payload, padded
    "ncrc", // 15 bool           If true, disable the CRC of the physical layer (optional)
};

static inline int getMetadataTxNameIndex(
    const char *name
)
{
    int r = 0;
    for (int i = 0; i < 16; i++) {
        if (strcmp(SAX_METADATA_TX_NAMES[i], name) == 0)
            return i;
    }
    return r;
}

class SaxPullResp : public nlohmann::json::json_sax_t {
private:
    int nameIndex;
    int startItem;  // object enter/exit counter
    GwPullResp *item;
public:
    int parseError;
    explicit SaxPullResp(
        GwPullResp *retVal,
        const DEVEUI &gwId
    )
        : nameIndex(0), startItem(0), item(retVal), parseError(CODE_OK)
    {
        item->gwId = gwId;
    }

    bool null() override {
        return true;
    }

    bool boolean(bool val) override {
        switch (nameIndex) {
            case 1: // "imme" Send packet immediately (will ignore tmst & time)
                item->txMetadata.tx_mode = 0;    // immediate
                break;
            case 11: // "ipol" Lora modulation polarization inversion
                item->txMetadata.invert_pol = val;
                break;
            case 15: // "ncrc" disable the CRC of the physical layer
                item->txMetadata.no_crc = val;
                break;
            default:
                break;
        }
        return true;
    }

    bool number_integer(number_integer_t val) override {
        return true;
    }

    bool number_unsigned(number_unsigned_t val) override {
        switch (nameIndex) {
            case 2: // "tmst" Send packet on a certain timestamp value (will ignore time)
                item->txMetadata.count_us = gps2utc(val);
                break;
            case 3: // "tmms" Send packet at a certain GPS time (GPS synchronization required)
                item->txMetadata.count_us = val;
                break;
            case 4: // "freq"TX central frequency in MHz (Hz precision)
                item->txMetadata.freq_hz = val * 1000000;
                break;
            case 5: // "rfch" Concentrator "RF chain" used for TX
                item->txMetadata.rf_chain = (uint8_t) val;
                break;
            case 6: // "powe" TX output power in dBm (dBm precision)
                item->txMetadata.rf_power = (int8_t) val;
                break;
            case 8: // "datr" MODULATION_FSK data rate in bits per second
                item->txMetadata.datarate = val; // bits pre second FSK
                break;
            case 10: // "fdev" MODULATION_FSK frequency deviation in Hz
                item->txMetadata.f_dev = val;
                break;
            case 12: // "prea" RF preamble size
                item->txMetadata.preamble = val;
                break;
            // case 13: // "size" RF packet payload size in bytes
            //    break;
            default:
                break;
        }
        return true;
    }

    bool number_float(number_float_t val, const string_t &s) override {
        switch (nameIndex) {
            case 4: // "freq" TX central frequency in MHz (Hz precision)
                item->txMetadata.freq_hz = (uint32_t) val * 1000000;
                break;
            default:
                break;
        }
        return true;
    }

    bool string(string_t &val) override {
        switch (nameIndex) {
            case 7: // "modu" Modulation identifier "MODULATION_LORA" or "MODULATION_FSK" MOD_UNDEFINED 0, MOD_LORA 0x10, MOD_FSK 0x20
                item->txMetadata.modulation = string2MODULATION(val.c_str());
                break;
            case 8: // "datr" LoRa data rate identifier e.g. SF12BW500
            {
                BANDWIDTH b;
                item->txMetadata.datarate = string2datr(b, val);
                item->txMetadata.bandwidth = b;
            }
                break;
            case 9: // "codr" LoRa ECC coding rate identifier
            {
                item->txMetadata.coderate = string2codingRate(val);

            }
                break;
            case 14: // data
                base64SetToLORAWAN_MESSAGE_STORAGE(item->txData, val);
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
        nameIndex = getMetadataTxNameIndex(val.c_str());
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

int GatewayBasicUdpProtocol::parse(
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
        {
            auto *pGw = (SEMTECH_PREFIX_GW *) packetForwarderPacket;
            ntoh_SEMTECH_PREFIX_GW(*pGw);
            r = parsePushData(&retVal.gwPushData, (char *) packetForwarderPacket + SIZE_SEMTECH_PREFIX_GW,
                size - SIZE_SEMTECH_PREFIX_GW, pGw->mac, receivedTime); // +12 bytes
        }
            break;
        case SEMTECH_GW_PULL_RESP:  // 4
        {
            auto *pGw = (SEMTECH_PREFIX_GW *) packetForwarderPacket;
            ntoh_SEMTECH_PREFIX_GW(*pGw);
            r = parsePullResp(&retVal.gwPullResp, (char *) packetForwarderPacket + SIZE_SEMTECH_PREFIX, size - SIZE_SEMTECH_PREFIX,
                pGw->mac); // +4 bytes

        }
            break;
        case SEMTECH_GW_TX_ACK:     // 5 gateway inform network server about does PULL_RESP data transmission was successful or not
            r = parseTxAck(&retVal.code, (char *) packetForwarderPacket + SIZE_SEMTECH_PREFIX_GW, SIZE_SEMTECH_PREFIX_GW); // +12 bytes
            break;
        default:
            r = ERR_CODE_INVALID_PACKET;
    }
    return r;
}

int parsePushData(
    GwPushData *retVal,
    const char *json,
    size_t size,
    const DEVEUI &gwId,
    TASK_TIME receivedTime
) {
    SaxPushData consumer(retVal, gwId, receivedTime);
    nlohmann::json::sax_parse(json, json + size, &consumer);
    return consumer.parseError;
}

int parsePullResp(
    GwPullResp *retVal,
    const char *json,
    size_t size,
    const DEVEUI &gwId
) {
    SaxPullResp consumer(retVal, gwId);
    nlohmann::json::sax_parse(json, json + size, &consumer);
    return consumer.parseError;
}

int parseTxAck(
    ERR_CODE_TX *retVal,
    const char *json,
    size_t size
) {
    nlohmann::json js = nlohmann::json::parse(json, json + size);
    if (!js.is_object())
        return ERR_CODE_INVALID_JSON;
    if (!js.contains(SAX_METADATA_TXPK_ACK_NAMES[0]))
        return ERR_CODE_INVALID_JSON;
    auto jAck = js[SAX_METADATA_TXPK_ACK_NAMES[0]];
    if (!jAck.is_object())
        return ERR_CODE_INVALID_JSON;
    if (!js.contains(SAX_METADATA_TXPK_ACK_NAMES[1]))
        return ERR_CODE_INVALID_JSON;
    auto jError = js[SAX_METADATA_TXPK_ACK_NAMES[1]];
    if (!jError.is_string())
        return ERR_CODE_INVALID_JSON;
    *retVal = string2ERR_CODE_TX(jError);
    return CODE_OK;
}

GatewayBasicUdpProtocol::GatewayBasicUdpProtocol(
    MessageTaskDispatcher *aDispatcher
)
    : ProtoGwParser(aDispatcher)
{

}

ssize_t GatewayBasicUdpProtocol::ack(
    char *retBuf,
    size_t retSize,
    const char *packet,
    size_t packetSize
)
{
    if (packetSize < SIZE_SEMTECH_ACK)
        return ERR_CODE_SEND_ACK;
    memmove(retBuf, packet, SIZE_SEMTECH_ACK);
    if (((SEMTECH_ACK *) retBuf)->version != 2)
        return ERR_CODE_SEND_ACK;
    ((SEMTECH_ACK *) retBuf)->tag++;
    return SIZE_SEMTECH_ACK;
}

bool GatewayBasicUdpProtocol::makeMessage2GatewayStream(
    std::ostream &ss,
    MessageBuilder &msgBuilder,
    uint16_t token,
    const SEMTECH_PROTOCOL_METADATA_RX *rxMetadata,
    const RegionalParameterChannelPlan *aRegionalPlan
)
{
    if (!rxMetadata)
        return false;
    SEMTECH_PREFIX pullPrefix { 2, token, SEMTECH_GW_PULL_DATA };
    ss << std::string((const char *) &pullPrefix, sizeof(SEMTECH_PREFIX))
       << "{\"" << SAX_METADATA_TX_NAMES[0] << "\":{"; // txpk
    // tmst
    if (rxMetadata->tmst) {
        ss << "\"" << SAX_METADATA_TX_NAMES[2] << "\":" << tmstAddMS(rxMetadata->tmst, 1000);
    } else {
        ss << "\"" << SAX_METADATA_TX_NAMES[1] << "\":true";    // send immediately
    }
    std::string radioPacketBase64 = msgBuilder.base64();
    ss << ",\"" << SAX_METADATA_TX_NAMES[4] << "\":" << freq2string(rxMetadata->freq)       // "868.900"
       // "rfch": 0. @see https://github.com/brocaar/chirpstack-network-server/issues/19
       << ",\"" << SAX_METADATA_TX_NAMES[5] << "\":" << 0                                      // Concentrator "RF chain" used for TX (unsigned integer)
       << ",\"" << SAX_METADATA_TX_NAMES[6] << "\":" << gwPower(rxMetadata, aRegionalPlan)									// TX output power in dBm (unsigned integer, dBm precision)
       << ",\"" << SAX_METADATA_TX_NAMES[7] << "\":\"" << MODULATION2String(rxMetadata->modu)	// Modulation identifier "LORA" or "FSK"
       << "\",\"" << SAX_METADATA_TX_NAMES[8] << "\":\"" << DATA_RATE2string(rxMetadata->bandwidth, rxMetadata->spreadingFactor)
       << "\",\"" << SAX_METADATA_TX_NAMES[9] << "\":\"" << codingRate2string(rxMetadata->codingRate)
       << "\",\"" << SAX_METADATA_TX_NAMES[11] << "\":true" // Lora modulation polarization inversion
       << ",\"" << SAX_METADATA_TX_NAMES[15] << "\":false"  // Check CRC
       << ",\"" << SAX_METADATA_TX_NAMES[13] << "\":" << msgBuilder.size();
    if (!radioPacketBase64.empty())
       ss << ",\"" << SAX_METADATA_TX_NAMES[14] << "\":\"" << radioPacketBase64;
    ss << "\"}}";
    return true;
}

ssize_t GatewayBasicUdpProtocol::makeMessage2Gateway(
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

const char *GatewayBasicUdpProtocol::name() const
{
    return GATEWAY_BASIC_UDP_PROTOCOL_NAME;
}

int GatewayBasicUdpProtocol::tag() const
{
    return 1;
}
