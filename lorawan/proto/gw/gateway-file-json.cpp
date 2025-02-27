#include <iomanip>
#include <sstream>
#include <cmath>
#include <algorithm>

#include "lorawan/proto/gw/gateway-file-json.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-error.h"

#define DEFAULT_BEACON_POWER        14
#define DEFAULT_BEACON_BW_HZ        125000
#define DEFAULT_BEACON_FREQ_HZ      869525000
#define DEFAULT_BEACON_DATARATE     9
#define DEFAULT_BEACON_FREQ_NB      1
#define DEFAULT_KEEPALIVE           5   // default time interval for downstream keep-alive packet

GatewayJsonConfig::GatewayJsonConfig()
    : errorCode(0)
{

}

int GatewayJsonConfig::parseString(
    const std::string &json
)
{
    errorCode = CODE_OK;
    nlohmann::json doc = nlohmann::json::parse(json, nullptr, false, true);
    if (doc.is_discarded()) {
        errorCode = ERR_CODE_INVALID_JSON;
        return errorCode;
    }
    return parse(doc);
}

std::string GatewayJsonConfig::toString() const
{
    nlohmann::json doc;
    toJSON(doc);
    return doc.dump();
}

GatewaySX1261Config::GatewaySX1261Config()
{
    reset();
}

void GatewaySX1261Config::reset()
{
    memset(&value.sx1261, 0, sizeof(struct lgw_conf_sx1261_s));
    memset(&value.spectralScan, 0, sizeof(spectral_scan_t));
    memset(&value.lbt, 0, sizeof(struct lgw_conf_lbt_s));
}

bool GatewaySX1261Config::operator==(
    const GatewaySX1261Config &b
) const
{
    if (value.lbt.nb_channel != b.value.lbt.nb_channel)
        return false;
    return (memcmp(&value.sx1261, &b.value.sx1261, sizeof(struct lgw_conf_sx1261_s)) == 0)
        && (memcmp(&value.spectralScan, &b.value.spectralScan, sizeof(spectral_scan_t)) == 0)
        && (memcmp(&value.lbt.channels, &b.value.lbt.channels, value.lbt.nb_channel * sizeof(struct lgw_conf_chan_lbt_s)) == 0);
}

int GatewaySX1261Config::parse(nlohmann::json &jsonValue)
{
    auto jEnable = jsonValue.find("enable");
    if (jEnable != jsonValue.end()) {
        if (jEnable->is_boolean()) {
            value.sx1261.enable = *jEnable;
        }
    }

    auto jSpiPath = jsonValue.find("spi_path");
    if (jSpiPath != jsonValue.end()) {
        if (jSpiPath->is_string()) {
            std::string s = *jSpiPath;
            size_t sz = s.size();
            if (sz < 64) {
                strncpy(&value.sx1261.spi_path[0], s.c_str(), 64);
                value.sx1261.spi_path[sz] = 0;
            }
        }
    }
    auto jRssiOffset = jsonValue.find("rssi_offset");
    if (jRssiOffset != jsonValue.end()) {
        if (jRssiOffset->is_number_integer()) {
            value.sx1261.rssi_offset = *jRssiOffset;
        }
    }
    auto jSpectralScan = jsonValue.find("spectral_scan");
    if (jSpectralScan != jsonValue.end()) {
        if (jSpectralScan->is_object()) {
            auto jEnable = jSpectralScan->find("enable");
            if (jEnable != jSpectralScan->end()) {
                if (jEnable->is_boolean()) {
                    value.spectralScan.enable = *jEnable;
                }
                if (value.spectralScan.enable) {
                    value.sx1261.enable = true;
                }
                auto jFreqStart = jSpectralScan->find("freq_start");
                if (jFreqStart != jSpectralScan->end()) {
                    if (jFreqStart->is_number_unsigned()) {
                        value.spectralScan.freq_hz_start = *jFreqStart;
                    }
                }
                auto jNbChan = jSpectralScan->find("nb_chan");
                if (jNbChan != jSpectralScan->end()) {
                    if (jNbChan->is_number_unsigned()) {
                        value.spectralScan.nb_chan = *jNbChan;
                    }
                }
                auto jNbScan = jSpectralScan->find("nb_scan");
                if (jNbScan != jSpectralScan->end()) {
                    if (jNbScan->is_number_unsigned()) {
                        value.spectralScan.nb_scan = *jNbScan;
                    }
                }
                auto jPaceS = jSpectralScan->find("pace_s");
                if (jPaceS != jSpectralScan->end()) {
                    if (jPaceS->is_number_unsigned()) {
                        value.spectralScan.pace_s = *jPaceS;
                    }
                }
            }
        }
    }
    // set LBT channels configuration
    auto jLbt = jsonValue.find("lbt");
    if (jLbt != jsonValue.end()) {
        if (jLbt->is_object()) {
            auto jLbtEnable = jsonValue.find("enable");
            if (jLbtEnable != jLbt->end()) {
                if (jLbtEnable->is_boolean()) {
                    value.lbt.enable = *jLbtEnable;
                }
                // Enable the sx1261 radio hardware configuration to allow spectral scan
                if (value.lbt.enable)
                    value.sx1261.enable = true;

                auto jRssiTarget = jsonValue.find("rssi_target");
                if (jRssiTarget != jLbt->end()) {
                    if (jRssiTarget->is_number_integer()) {
                        value.lbt.rssi_target = *jRssiTarget;
                    }
                }

                // set LBT channels configuration
                auto jChannels = jLbt->find("channels");
                if (jChannels != jLbt->end()) {
                    if (jChannels->is_array()) {
                        value.lbt.nb_channel = jChannels->size();
                        if (value.lbt.nb_channel > LGW_LBT_CHANNEL_NB_MAX)
                            value.lbt.nb_channel = LGW_LBT_CHANNEL_NB_MAX;
                        for (int i = 0; i < value.lbt.nb_channel; i++) {
                            nlohmann::json jChannel = jChannels[i];
                            auto jFreqHz = jChannel.find("freq_hz");
                            if (jFreqHz != jChannel.end()) {
                                if (jFreqHz->is_number_unsigned()) {
                                    value.lbt.channels[i].freq_hz = *jFreqHz;
                                }
                            }
                            auto jBandwidth = jChannel.find("bandwidth");
                            if (jBandwidth != jChannel.end()) {
                                if (jBandwidth->is_number_unsigned()) {
                                    uint32_t bw = *jBandwidth;
                                    switch(bw) {
                                        case 500000:
                                            value.lbt.channels[i].bandwidth = BW_500KHZ;
                                            break;
                                        case 250000:
                                            value.lbt.channels[i].bandwidth = BW_250KHZ;
                                            break;
                                        case 125000:
                                            value.lbt.channels[i].bandwidth = BW_125KHZ;
                                            break;
                                        default:
                                            value.lbt.channels[i].bandwidth = BW_UNDEFINED;
                                    }
                                }
                            }
                            auto jScanTime = jChannel.find("scan_time_us");
                            if (jScanTime != jChannel.end()) {
                                if (jScanTime->is_number_unsigned()) {
                                    switch ((unsigned ) *jScanTime) {
                                        case LGW_LBT_SCAN_TIME_128_US:
                                            value.lbt.channels[i].scan_time_us = LGW_LBT_SCAN_TIME_128_US;
                                            break;
                                        default:
                                            value.lbt.channels[i].scan_time_us = LGW_LBT_SCAN_TIME_5000_US;
                                    }
                                }
                            }
                            auto jTransmit = jChannel.find("transmit_time_ms");
                            if (jTransmit != jChannel.end()) {
                                if (jTransmit->is_number_unsigned()) {
                                    value.lbt.channels[i].transmit_time_ms = *jTransmit;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return CODE_OK;
}

static int bandwidthIndex2hz(
        uint8_t bandwidthIndex
) {
    switch(bandwidthIndex) {
        case BW_500KHZ:
            return 500000;
        case BW_250KHZ:
            return 250000;
        case BW_125KHZ:
            return 125000;
            // TODO ?!!
        case 3:
            return 62000;
        case 2:
            return 41000;
        case 1:
            return 31000;
        case 0:
            return 20000;
        default:
            return 15000;
    }
}

void GatewaySX1261Config::toHeader(
    std::ostream &retVal,
    const std::string &name,
    bool cpp20
) const
{
    if (cpp20) {
        retVal << "\t// " << name << "\n\t\t.sx1261 = {\n"
            "\t\t\t.sx1261 = {\n"
            "\t\t\t.enable = " << (value.sx1261.enable ? "true" : "false") << ",\n"
            "\t\t\t\t.spi_path = \"" << value.sx1261.spi_path << "\",\n"
            "\t\t\t\t.rssi_offset = " << (int) value.sx1261.rssi_offset << "\n"
            "\t\t\t},\n"
            "\t\t\t.spectralScan = {\n"
            "\t\t\t\t.enable = " << (value.spectralScan.enable ? "true" : "false") << ",\n"
            "\t\t\t\t.freq_hz_start = " << value.spectralScan.freq_hz_start << ",\n"
            "\t\t\t\t.nb_chan = " << (int) value.spectralScan.nb_chan << ",\n"
            "\t\t\t\t.nb_scan = " << value.spectralScan.nb_scan << ",\n"
            "\t\t\t\t.pace_s = " << value.spectralScan.pace_s << "\n"
            "\t\t\t},\n"
            "\t\t\t.lbt = {\n"
            "\t\t\t\t.enable = " << (value.lbt.enable ? "true" : "false") << ",\n"
            "\t\t\t\t.nb_channel = " << (int) value.lbt.nb_channel;
        if (value.lbt.nb_channel)
            retVal << ",";
        retVal << "\n";
        if (value.lbt.nb_channel) {
            retVal << "\t\t\t\t.channels = {\n";
            bool isFirst = true;
            for (int i = 0; i < value.lbt.nb_channel; i++) {
                if (isFirst)
                    isFirst = false;
                else
                    retVal << ",\n";
                retVal <<
                    "\t\t\t\t\t{\n"
                    "\t\t\t\t\t\t.freq_hz = " << value.lbt.channels[i].freq_hz << ",\n"
                    "\t\t\t\t\t\t.bandwidth = " << (int) value.lbt.channels[i].bandwidth << ",\n"
                    "\t\t\t\t\t\t.scan_time_us = (lgw_lbt_scan_time_t) " << (int) value.lbt.channels[i].scan_time_us << ",\n"
                    "\t\t\t\t\t\t.transmit_time_ms = " << value.lbt.channels[i].transmit_time_ms << "\n"
                "\t\t\t\t\t}\n";
            }
            retVal << "\t\t\t\t},\n";
        }
        retVal << "\t\t\t}\n\t\t}";
    } else {
        retVal << "\t// " << name << "\n\t{ // sx1261\n"
        "\t\t{ // sx1261\n"
        "\t\t\t" << (value.sx1261.enable ? "true" : "false") << ", // enable\n"
        "\t\t\t\"" << value.sx1261.spi_path << "\", // spi_path\n"
        "\t\t\t" << (int) value.sx1261.rssi_offset << ", // rssi_offset\n"
        "\t\t\t{ // lbt_conf\n"
        "\t\t\t\t" << (value.sx1261.lbt_conf.enable ? "true" : "false") << ", // enable\n"
        "\t\t\t\t" << (int) value.sx1261.lbt_conf.rssi_target << ", // rssi_target\n"
        "\t\t\t\t" << (int) value.sx1261.lbt_conf.nb_channel << ", // nb_channel\n"
        "\t\t\t\t{ // channels\n";
        bool isFirst = true;
        for (int nbc = 0; nbc < value.sx1261.lbt_conf.nb_channel; nbc) {
            if (isFirst)
                isFirst = false;
            else
                retVal << ",\n";
            retVal << "\t\t\t\t\t{, // channels[" << nbc << "]\n"
                << "\t\t\t\t\t\t" << value.sx1261.lbt_conf.channels[nbc].freq_hz << ", // freq_hz \n"
                << "\t\t\t\t\t\t" << (int) value.sx1261.lbt_conf.channels[nbc].bandwidth << ", // bandwidth\n"
                << "\t\t\t\t\t\t" << (int) value.sx1261.lbt_conf.channels[nbc].scan_time_us << ", // scan_time_us\n"
                << "\t\t\t\t\t\t" << value.sx1261.lbt_conf.channels[nbc].transmit_time_ms << ", // transmit_time_ms\n"
                << "\t\t\t\t\t}";
        }
        retVal << "\t\t\t\t}\n"
        << "\t\t\t}\n"
        "\t\t},\n"

        "\t\t{ // spectralScan\n"
        "\t\t\t" << (value.spectralScan.enable ? "true" : "false") << ", // enable\n"
        "\t\t\t" << value.spectralScan.freq_hz_start << ", // freq_hz_start\n"
        "\t\t\t" << (int) value.spectralScan.nb_chan << ", // nb_chan\n"
        "\t\t\t" << value.spectralScan.nb_scan << ", // nb_scan\n"
        "\t\t\t" << value.spectralScan.pace_s << " // pace_s\n"
        "\t\t},\n"
        "\t\t{ // lbt\n"
        "\t\t\t" << (value.lbt.enable ? "true" : "false") << ", // enable\n"
        "\t\t\t" << (int) value.lbt.rssi_target << ", // rssi_target\n"
        "\t\t\t" << (int) value.lbt.nb_channel << ", // nb_channel\n"
        "\t\t\t{ // channels\n";
        isFirst = true;
        for (int i = 0; i < value.lbt.nb_channel; i++) {
            if (isFirst)
                isFirst = false;
            else
                retVal << ",\n";
            retVal <<
               "\t\t\t\t{ // channels[" << i <<"]\n"
               "\t\t\t\t\t" << value.lbt.channels[i].freq_hz << ", // freq_hz\n"
               "\t\t\t\t\t" << (int) value.lbt.channels[i].bandwidth << ", // bandwidth\n"
               "\t\t\t\t\t(lgw_lbt_scan_time_t) " << (int) value.lbt.channels[i].scan_time_us << ", // scan_time_us\n"
               "\t\t\t\t" << value.lbt.channels[i].transmit_time_ms << ", // transmit_time_ms\n"
               "\t\t\t\t}\n";
        }
        retVal << "\t\t\t}\n\t\t}\n\t}";
    }
}

void GatewaySX1261Config::toJSON(
    nlohmann::json &jsonValue
) const {
    jsonValue["spi_path"] = value.sx1261.spi_path;
    jsonValue["rssi_offset"] = value.sx1261.rssi_offset;
    jsonValue["rssi_offset"] = value.sx1261.rssi_offset;

    nlohmann::json ss;
    ss["enable"] = value.spectralScan.enable;
    ss["freq_start"] = value.spectralScan.freq_hz_start;
    ss["nb_chan"] = value.spectralScan.nb_chan;
    ss["nb_scan"] = value.spectralScan.nb_scan;
    ss["pace_s"] = value.spectralScan.pace_s;
    jsonValue["spectral_scan"] = ss;

    nlohmann::json lbt;
    lbt["enable"] = value.lbt.enable;
    lbt["rssi_target"] = value.lbt.rssi_target;

    nlohmann::json channels;
    for (int i = 0; i < value.lbt.nb_channel; i++) {
        nlohmann::json channel;
        channel["freq_hz"] = value.lbt.channels[i].freq_hz;
        channel["bandwidth"] = bandwidthIndex2hz(value.lbt.channels[i].bandwidth);
        channel["scan_time_us"] = value.lbt.channels[i].scan_time_us;
        channel["transmit_time_ms"] = value.lbt.channels[i].transmit_time_ms;
        channels.push_back(channel);
    }
    lbt["channels"] = channels;
    jsonValue["lbt"] = lbt;
}

GatewaySX130xConfig::GatewaySX130xConfig()
{
    reset();
}

void GatewaySX130xConfig::reset()
{
    sx1261Config.reset();
    value.antennaGain = 0;
    memset(&value.boardConf, 0, sizeof(struct lgw_conf_board_s));
    memset(&value.tsConf, 0, sizeof(struct lgw_conf_ftime_s));
    for (int i = 0; i < LGW_RF_CHAIN_NB; i++) {
        value.tx_freq_min[i] = 0;
        value.tx_freq_max[i] = 0;
        memset(&value.rfConfs[i], 0, sizeof(struct lgw_conf_rxrf_s));
        memset(&value.txLut[i], 0, sizeof(struct lgw_tx_gain_lut_s));
    }
    for (auto & ifConf : value.ifConfs) {
        memset(&ifConf, 0, sizeof(struct lgw_conf_rxif_s));
    }
    memset(&value.ifStdConf, 0, sizeof(struct lgw_conf_rxif_s));
    memset(&value.ifFSKConf, 0, sizeof(struct lgw_conf_rxif_s));
    memset(&value.demodConf, 0, sizeof(struct lgw_conf_demod_s));
}

bool GatewaySX130xConfig::operator==(
    const GatewaySX130xConfig &b
) const
{
    for (int i = 0; i < LGW_RF_CHAIN_NB; i++) {
        if (memcmp(&value.rfConfs[i], &b.value.rfConfs[i], sizeof(struct lgw_conf_rxrf_s)))
            return false;

        if (memcmp(&value.txLut[i], &b.value.txLut[i], sizeof(struct lgw_tx_gain_lut_s)))
            return false;
        if (value.tx_freq_min[i] != b.value.tx_freq_min[i])
            return false;
        if (value.tx_freq_max[i] != b.value.tx_freq_max[i])
            return false;
    }

    for (int i = 0; i < LGW_MULTI_NB; i++) {
        if (memcmp(&value.ifConfs[i], &b.value.ifConfs[i], sizeof(struct lgw_conf_rxif_s)))
            return false;
    }

    return
        (sx1261Config == b.sx1261Config)
        && (value.antennaGain == b.value.antennaGain)
        // && (memcmp(&boardConf, &b.boardConf, sizeof(struct lgw_conf_board_s)) == 0)
        && value.boardConf.lorawan_public == b.value.boardConf.lorawan_public
        && value.boardConf.clksrc == b.value.boardConf.clksrc
        && value.boardConf.full_duplex == b.value.boardConf.full_duplex
        && value.boardConf.com_type == b.value.boardConf.com_type
        && strncmp(value.boardConf.com_path, b.value.boardConf.com_path, 64) == 0

        && (memcmp(&value.ifStdConf, &b.value.ifStdConf, sizeof(struct lgw_conf_rxif_s)) == 0)
        && (memcmp(&value.ifFSKConf, &b.value.ifFSKConf, sizeof(struct lgw_conf_rxif_s)) == 0)
        && (memcmp(&value.demodConf, &b.value.demodConf, sizeof(struct lgw_conf_demod_s)) == 0)
        && (memcmp(&value.tsConf, &b.value.tsConf, sizeof(struct lgw_conf_ftime_s)) == 0);
}

std::string GatewaySX130xConfig::getUsbPath()
{
    if (value.boardConf.com_path[63] == '\0')
        return std::string(value.boardConf.com_path);
    else
        return std::string(value.boardConf.com_path, 64);
}

void GatewaySX130xConfig::setUsbPath(const std::string &val)
{
    size_t sz = val.size();
    if (sz > 64)
        sz = 64;
    strncpy(value.boardConf.com_path, val.c_str(), sz);
}

int GatewaySX130xConfig::parse(nlohmann::json &jsonValue) {
    reset();
    auto jSx1261 = jsonValue.find("sx1261_conf");
    if (jSx1261 != jsonValue.end()) {
        int r = sx1261Config.parse(*jSx1261);
        if (r)
            return r;
    }

    value.boardConf.com_type = LGW_COM_UNKNOWN;
    auto jCom_type = jsonValue.find("com_type");
    if (jCom_type != jsonValue.end()) {
        if (jCom_type->is_string()) {
            std::string s = *jCom_type;
            if (s == "SPI" || s == "spi")
                value.boardConf.com_type = LGW_COM_SPI;
            if (s == "USB" || s == "usb")
                value.boardConf.com_type = LGW_COM_USB;
        }
    }
    if (value.boardConf.com_type == LGW_COM_UNKNOWN)
        return ERR_CODE_INVALID_JSON;

    auto jComPath = jsonValue.find("com_path");
    if (jComPath != jsonValue.end()) {
        if (jComPath->is_string()) {
            std::string s = *jComPath;
            size_t sz = s.size();
            if (sz < 64) {
                strncpy(&value.boardConf.com_path[0], s.c_str(), 64);
                value.boardConf.com_path[sz] = 0;
            }
        }
    }

    auto jLorawanPublic = jsonValue.find("lorawan_public");
    if (jLorawanPublic != jsonValue.end()) {
        if (jLorawanPublic-> is_boolean()) {
            value.boardConf.lorawan_public = *jLorawanPublic;
        }
    }

    auto jClcSrc = jsonValue.find("clksrc");
    if (jClcSrc != jsonValue.end()) {
        if (jClcSrc->is_number_unsigned()) {
            value.boardConf.clksrc = *jClcSrc;
        }
    }

    auto jFullDuplex = jsonValue.find("full_duplex");
    if (jFullDuplex != jsonValue.end()) {
        if (jFullDuplex->is_boolean()) {
            value.boardConf.full_duplex = *jFullDuplex;
        }
    }

    auto jAntennaGain = jsonValue.find("antenna_gain");
    if (jAntennaGain != jsonValue.end()) {
        if (jAntennaGain->is_number_unsigned()) {
            value.antennaGain = *jAntennaGain;
        }
    }

    auto jFineTimestamp = jsonValue.find("fine_timestamp");
    if (jFineTimestamp != jsonValue.end()) {
        if (jFineTimestamp->is_object()) {
            auto jEnable = jFineTimestamp->find("enable");
            if (jEnable != jFineTimestamp->end()) {
                if (jEnable->is_boolean()) {
                    value.tsConf.enable = *jEnable;
                }
                value.tsConf.mode = LGW_FTIME_MODE_HIGH_CAPACITY;
                if (value.tsConf.enable) {
                    auto jMode = jFineTimestamp->find("mode");
                    if (jMode != jFineTimestamp->end()) {
                        if (jMode->is_string()) {
                            std::string s = *jMode;
                            if (s == "all_sf" || s == "ALL_SF") {
                                value.tsConf.mode = LGW_FTIME_MODE_ALL_SF;
                            }
                            if (s == "high_capacity" || s == "HIGH_CAPACITY") {
                                value.tsConf.mode = LGW_FTIME_MODE_HIGH_CAPACITY;
                            }
                        }
                    }
                }
            }
        }
    }

    std::string rn = "radio_0";
    for (int radioIndex = 0; radioIndex < LGW_RF_CHAIN_NB; radioIndex++) {
        std::stringstream ssRadioName;
        rn[6] = '0' + radioIndex;
        auto jRadio = jsonValue.find(rn);
        if (jRadio != jsonValue.end()) {
            if (jRadio->is_object()) {\
                auto jRadioEnable = jRadio->find("enable");
                if (jRadioEnable != jRadio->end()) {
                    if (jRadioEnable->is_boolean()) {
                        value.rfConfs[radioIndex].enable = *jRadioEnable;
                    }
                    value.rfConfs[radioIndex].type = LGW_RADIO_TYPE_NONE;
                    if (value.rfConfs[radioIndex].enable) {
                        auto jType = jRadio->find("type");
                        if (jType != jRadio->end()) {
                            if (jType->is_string()) {
                                std::string s = *jType;
                                if (s == "SX1255") {
                                    value.rfConfs[radioIndex].type = LGW_RADIO_TYPE_SX1255;
                                }
                                if (s == "SX1257") {
                                    value.rfConfs[radioIndex].type = LGW_RADIO_TYPE_SX1257;
                                }
                                if (s == "SX1250") {
                                    value.rfConfs[radioIndex].type = LGW_RADIO_TYPE_SX1250;
                                }
                            }
                        }
                        auto jFreq = jRadio->find("freq");
                        if (jFreq != jRadio->end()) {
                            if (jFreq->is_number_unsigned()) {
                                value.rfConfs[radioIndex].freq_hz = *jFreq;
                            }
                        }
                        auto jRssiOffset = jRadio->find("rssi_offset");
                        if (jRssiOffset != jRadio->end()) {
                            if (jRssiOffset->is_number_integer()) {
                                value.rfConfs[radioIndex].rssi_offset = *jRssiOffset;
                            }
                            if (jRssiOffset->is_number_float()) {
                                value.rfConfs[radioIndex].rssi_offset = *jRssiOffset;
                            }
                        }

                        auto jTComp = jRadio->find("rssi_tcomp");
                        if (jTComp != jRadio->end()) {
                            if (jTComp->is_object()) {
                                auto jCoeffA = jTComp->find("coeff_a");
                                if (jCoeffA != jTComp->end()) {
                                    if (jCoeffA->is_number_unsigned()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_a = *jCoeffA;
                                    }
                                    if (jCoeffA->is_number_float()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_a = *jCoeffA;
                                    }
                                }
                                auto jCoeffB = jTComp->find("coeff_b");
                                if (jCoeffB != jTComp->end()) {
                                    if (jCoeffB->is_number_unsigned()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_b = *jCoeffB;
                                    }
                                    if (jCoeffB->is_number_float()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_b = *jCoeffB;
                                    }
                                }
                                auto jCoeffC = jTComp->find("coeff_c");
                                if (jCoeffC != jTComp->end()) {
                                    if (jCoeffC->is_number_unsigned()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_c = *jCoeffC;
                                    }
                                    if (jCoeffC->is_number_float()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_c = *jCoeffC;
                                    }
                                }
                                auto jCoeffD = jTComp->find("coeff_d");
                                if (jCoeffD != jTComp->end()) {
                                    if (jCoeffD->is_number_unsigned()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_d = *jCoeffD;
                                    }
                                    if (jCoeffD->is_number_float()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_d = *jCoeffD;
                                    }
                                }
                                auto jCoeffE = jTComp->find("coeff_e");
                                if (jCoeffE != jTComp->end()) {
                                    if (jCoeffE->is_number_unsigned()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_e = *jCoeffE;
                                    }
                                    if (jCoeffE->is_number_float()) {
                                        value.rfConfs[radioIndex].rssi_tcomp.coeff_e = *jCoeffE;
                                    }
                                }
                            }
                        }
                        auto jSingleMode = jRadio->find("single_input_mode");
                        if (jSingleMode != jRadio->end()) {
                            if (jSingleMode->is_boolean()) {
                                value.rfConfs[radioIndex].single_input_mode = *jSingleMode;
                            }
                        }
                        auto jTxEnable = jRadio->find("tx_enable");
                        if (jTxEnable != jRadio->end()) {
                            if (jTxEnable->is_boolean()) {
                                value.rfConfs[radioIndex].tx_enable = *jTxEnable;
                            }
                        }
                        auto jTxFreqMin = jRadio->find("tx_freq_min");
                        if (jTxFreqMin != jRadio->end()) {
                            if (jTxFreqMin->is_number_unsigned()) {
                                value.tx_freq_min[radioIndex] = *jTxFreqMin;
                            }
                        }
                        auto jTxFreqMax = jRadio->find("tx_freq_max");
                        if (jTxFreqMax != jRadio->end()) {
                            if (jTxFreqMax->is_number_unsigned()) {
                                value.tx_freq_max[radioIndex] = *jTxFreqMax;
                            }
                        }
                        auto jGainLut = jRadio->find("tx_gain_lut");
                        if (jGainLut != jRadio->end()) {
                            if (jGainLut->is_array()) {
                                size_t sz = jGainLut->size();
                                if (sz > 16)
                                    sz = 16;
                                value.txLut[radioIndex].size = sz;

                                for (int i = 0; i < sz; i++) {
                                    nlohmann::json jGain = jGainLut->at(i);
                                    bool sx1250_tx_lut = false;
                                    if (jGain.is_object()) {
                                        auto jPwrIdx = jGain.find("pwr_idx");
                                        if (jPwrIdx != jGain.end()) {
                                            if (jPwrIdx->is_number_unsigned()) {
                                                value.txLut[radioIndex].lut[i].pwr_idx = *jPwrIdx;
                                                sx1250_tx_lut = true;
                                            }
                                        }
                                        if (sx1250_tx_lut) {
                                            value.txLut[radioIndex].lut[i].mix_gain = 5;
                                        }
                                        // else {
                                        auto jPower = jGain.find("rf_power");
                                        if (jPower != jGain.end()) {
                                            if (jPower->is_number_integer()) {
                                                value.txLut[radioIndex].lut[i].rf_power = *jPower;
                                            }
                                        }
                                        auto jPaGain = jGain.find("pa_gain");
                                        if (jPaGain != jGain.end()) {
                                            if (jPaGain->is_number_unsigned()) {
                                                value.txLut[radioIndex].lut[i].pa_gain = *jPaGain;
                                            }
                                        }
                                        auto jDigGain = jGain.find("dig_gain");
                                        if (jDigGain != jGain.end()) {
                                            if (jDigGain->is_number_unsigned()) {
                                                value.txLut[radioIndex].lut[i].dig_gain = *jDigGain;
                                            }
                                        }
                                        auto jDacGain = jGain.find("dac_gain");
                                        if (jDacGain != jGain.end()) {
                                            if (jDacGain->is_number_unsigned()) {
                                                value.txLut[radioIndex].lut[i].dac_gain = *jDacGain;
                                            }
                                        }
                                        auto jMixGain = jGain.find("mix_gain");
                                        if (jMixGain != jGain.end()) {
                                            if (jMixGain->is_number_unsigned()) {
                                                value.txLut[radioIndex].lut[i].mix_gain = *jMixGain;
                                            }
                                        }
                                        // } // else
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // demodulator
    // chan_multiSF_All
    bool hasSf = false;
    auto jDemod = jsonValue.find("chan_multiSF_All");
    if (jDemod != jsonValue.end()) {
        if (jDemod->is_object()) {
            auto jSFEnables = jDemod->find("spreading_factor_enable");
            if (jSFEnables != jDemod->end()) {
                if (jSFEnables->is_array()) {
                    size_t sz = jSFEnables->size();
                    value.demodConf.multisf_datarate = 0;
                    hasSf = true;
                    for (int i = 0; i < sz; i++) {
                        uint8_t n = jSFEnables->at(i);
                        value.demodConf.multisf_datarate |= (1 << (n - 5));
                    }
                }
            }
        }
    }
    if (!hasSf)
        value.demodConf.multisf_datarate = 0xff;  // all

    // set configuration for Lora multi-SF channels (bandwidth cannot be set)
    std::string cmsf = "chan_multiSF_0";
    for (int ch = 0; ch < LGW_MULTI_NB; ch++) {
        std::stringstream ssChannelName;
        cmsf[13] = '0' + ch;
        auto jChannelSF = jsonValue.find(cmsf);
        if (jChannelSF!= jsonValue.end()) {
            if (jChannelSF->is_object()) {
                auto jChannelEnable = jChannelSF->find("enable");
                if (jChannelEnable != jChannelSF->end()) {
                    if (jChannelEnable->is_boolean()) {
                        value.ifConfs[ch].enable = *jChannelEnable;
                    }
                    auto jChannelRadio = jChannelSF->find("radio");
                    if (jChannelRadio != jChannelSF->end()) {
                        if (jChannelRadio->is_number_unsigned()) {
                            value.ifConfs[ch].rf_chain = *jChannelRadio;
                        }
                    }
                    auto jChain = jChannelSF->find("if");
                    if (jChain != jChannelSF->end()) {
                        if (jChain->is_number_integer()) {
                            value.ifConfs[ch].freq_hz = *jChain;
                        }
                    }
                }
            }
        }
    }
    // set configuration for Lora standard channel
    value.ifStdConf.enable = false;
    auto jChannelStd = jsonValue.find("chan_Lora_std");
    if (jChannelStd != jsonValue.end()) {
        if (jChannelStd->is_object()) {
            auto jChannelEnable = jChannelStd->find("enable");
            if (jChannelEnable != jChannelStd->end()) {
                if (jChannelEnable->is_boolean()) {
                    value.ifStdConf.enable = *jChannelEnable;
                }
                auto jChannelRadio = jChannelStd->find("radio");
                if (jChannelRadio != jChannelStd->end()) {
                    if (jChannelRadio->is_number_unsigned()) {
                        value.ifStdConf.rf_chain = *jChannelRadio;
                    }
                }
                auto jFrequency = jChannelStd->find("if");
                if (jFrequency != jChannelStd->end()) {
                    if (jFrequency->is_number_integer()) {
                        value.ifStdConf.freq_hz = *jFrequency;
                    }
                }
                auto jBw = jChannelStd->find("bandwidth");
                if (jBw != jChannelStd->end()) {
                    if (jBw->is_number_unsigned()) {
                        uint32_t bw = *jBw;
                        switch(bw) {
                            case 500000:
                                value.ifStdConf.bandwidth = BW_500KHZ;
                                break;
                            case 250000:
                                value.ifStdConf.bandwidth = BW_250KHZ;
                                break;
                            case 125000:
                                value.ifStdConf.bandwidth = BW_125KHZ;
                                break;
                            default:
                                value.ifStdConf.bandwidth = BW_UNDEFINED;
                        }
                    }
                }
                auto jSF = jChannelStd->find("spread_factor");
                if (jSF != jChannelStd->end()) {
                    if (jSF->is_number_unsigned()) {
                        value.ifStdConf.datarate = *jSF;
                    }
                }
                auto jImplicitHeader = jChannelStd->find("implicit_hdr");
                if (jImplicitHeader != jChannelStd->end()) {
                    if (jImplicitHeader->is_boolean()) {
                        value.ifStdConf.implicit_hdr = *jImplicitHeader;
                    }
                    auto jImplicitPayloadLength = jChannelStd->find("implicit_payload_length");
                    if (jImplicitPayloadLength != jChannelStd->end()) {
                        if (jImplicitPayloadLength->is_number_unsigned()) {
                            value.ifStdConf.implicit_payload_length = *jImplicitPayloadLength;
                        }
                    }
                    auto jImplicitCrcEn = jChannelStd->find("implicit_crc_en");
                    if (jImplicitCrcEn != jChannelStd->end()) {
                        if (jImplicitCrcEn->is_boolean()) {
                            value.ifStdConf.implicit_crc_en = *jImplicitCrcEn;
                        }
                    }
                    auto jImplicitCoderate = jChannelStd->find("implicit_coderate");
                    if (jImplicitCoderate != jChannelStd->end()) {
                        if (jImplicitCrcEn->is_number_unsigned()) {
                            value.ifStdConf.implicit_coderate = *jImplicitCrcEn;
                        }
                    }
                }

            }
        }
    }

    // set configuration for FSK channel
    value.ifFSKConf.enable = false;
    auto jChannelFSK = jsonValue.find("chan_FSK");
    if (jChannelFSK != jsonValue.end()) {
        if (jChannelFSK->is_object()) {
            auto jChannelEnable = jChannelFSK->find("enable");
            if (jChannelEnable != jChannelFSK->end()) {
                if (jChannelEnable->is_boolean()) {
                    value.ifFSKConf.enable = *jChannelEnable;
                }
                auto jChannelRadio = jChannelFSK->find("radio");
                if (jChannelRadio != jChannelFSK->end()) {
                    if (jChannelRadio->is_number_integer()) {
                        value.ifFSKConf.rf_chain = *jChannelRadio;
                    }
                }
                auto jChain = jChannelFSK->find("if");
                if (jChain != jChannelFSK->end()) {
                    if (jChain->is_number_unsigned()) {
                        value.ifFSKConf.freq_hz = *jChain;
                    }
                }

                uint32_t bw = 0;
                auto jBw = jChannelFSK->find("bandwidth");
                if (jBw != jChannelFSK->end()) {
                    if (jBw->is_number_unsigned()) {
                        bw = *jBw;
                    }
                }

                uint32_t frequencyDeviation = 0;
                auto jFreqDeviation = jChannelFSK->find("freq_deviation");
                if (jFreqDeviation != jChannelFSK->end()) {
                    if (jFreqDeviation->is_number_unsigned()) {
                        frequencyDeviation = *jFreqDeviation;
                    }
                }

                auto jDataRate = jChannelFSK->find("datarate");
                if (jDataRate != jChannelFSK->end()) {
                    if (jDataRate->is_number_unsigned()) {
                        value.ifFSKConf.datarate = *jDataRate;
                    }
                }

                if ((bw == 0) && (frequencyDeviation != 0)) {
                    bw = 2 * frequencyDeviation + value.ifFSKConf.datarate;
                }
                if (bw == 0)
                    value.ifFSKConf.bandwidth = BW_UNDEFINED;
                else
                    if (bw <= 125000)
                        value.ifFSKConf.bandwidth = BW_125KHZ;
                    else
                        if (bw <= 250000)
                            value.ifFSKConf.bandwidth = BW_250KHZ;
                        else
                            if (bw <= 500000)
                                value.ifFSKConf.bandwidth = BW_500KHZ;
                            else
                                value.ifFSKConf.bandwidth = BW_UNDEFINED;
            }
        }
    }
    return 0;
}

static const char* lgw_com_type_t2string(lgw_com_type_t typ) {
    switch (typ) {
        case LGW_COM_SPI:
            return "SPI";
        case LGW_COM_USB:
            return "USB";
        default:
            return "unknown";
    }
}

static const char* lgw_ftime_mode_t2string(
    lgw_ftime_mode_t mode
) {
    switch (mode) {
        case LGW_FTIME_MODE_HIGH_CAPACITY:
            return "high_capacity";
        default:
            return "all_sf";
    }
}

static const char *lgw_radio_type_t2string(
    lgw_radio_type_t typ
) {
    switch (typ) {
        case LGW_RADIO_TYPE_SX1255:
            return "SX1255";
        case LGW_RADIO_TYPE_SX1257:
            return "SX1257";
        case LGW_RADIO_TYPE_SX1272:
            return "SX1272";
        case LGW_RADIO_TYPE_SX1276:
            return "SX1276";
        case LGW_RADIO_TYPE_SX1250:
            return "SX1250";
        default:
            return "none";
    }
}

void GatewaySX130xConfig::toHeader(
    std::ostream &retVal,
    const std::string &name,
    bool cpp20
) const
{
    if (cpp20) {
        retVal << "\t\t.sx130x = {\n"
            "\t\t\t.boardConf = {\n"
            "\t\t\t\t.lorawan_public = " << (value.boardConf.lorawan_public ? "true" : "false") << ",\n"
            "\t\t\t\t.clksrc = " << (int) value.boardConf.clksrc << ",\n"
            "\t\t\t\t.full_duplex = " << (value.boardConf.full_duplex ? "true" : "false") << ",\n"
            "\t\t\t\t.com_type = (lgw_com_type_t) " << (int) value.boardConf.com_type << ",\n"
            "\t\t\t\t.com_path = \"" << value.boardConf.com_path << "\"\n"
            "\t\t\t},\n"
            "\t\t\t.antennaGain = " << (int) value.antennaGain << ",\n"
            "\t\t\t.tsConf = {\n"
            "\t\t\t\t.enable = " << (value.tsConf.enable ? "true" : "false") << ",\n"
            "\t\t\t\t.mode = (lgw_ftime_mode_t) " << (int) value.tsConf.mode << "\n"
            "\t\t\t},\n"
            "\t\t\t.rfConfs = {";

        for (int radioIndex = 0; radioIndex < LGW_RF_CHAIN_NB; radioIndex++) {
            retVal << "\n\t\t\t\t// Radio " << radioIndex << "\n\t\t\t\t{\n"
                "\t\t\t\t\t.enable = " << (value.rfConfs[radioIndex].enable ? "true" : "false") << ",\n"
                "\t\t\t\t\t.freq_hz = " << value.rfConfs[radioIndex].freq_hz << ",\n"
                "\t\t\t\t\t.rssi_offset = " << value.rfConfs[radioIndex].rssi_offset << ",\n"
                "\t\t\t\t\t.rssi_tcomp = {\n"
                "\t\t\t\t\t\t.coeff_a = " << value.rfConfs[radioIndex].rssi_tcomp.coeff_a << ",\n"
                "\t\t\t\t\t\t.coeff_b = " << value.rfConfs[radioIndex].rssi_tcomp.coeff_b << ",\n"
                "\t\t\t\t\t\t.coeff_c = " << value.rfConfs[radioIndex].rssi_tcomp.coeff_c << ",\n"
                "\t\t\t\t\t\t.coeff_d = " << value.rfConfs[radioIndex].rssi_tcomp.coeff_d << ",\n"
                "\t\t\t\t\t\t.coeff_e = " << value.rfConfs[radioIndex].rssi_tcomp.coeff_e << "\n"
                "\t\t\t\t\t},\n"
                "\t\t\t\t\t.type = (lgw_radio_type_t) " << (int) value.rfConfs[radioIndex].type << ",\n"
                "\t\t\t\t\t.tx_enable = " << (value.rfConfs[radioIndex].tx_enable ? "true" : "false") << ",\n"
                "\t\t\t\t\t.single_input_mode = " << (value.rfConfs[radioIndex].single_input_mode ? "true" : "false") << "\n"
                "\t\t\t\t}";
            if (radioIndex < LGW_RF_CHAIN_NB - 1)
                retVal << ",";
        }
        retVal << "\n\t\t\t},\n\t\t\t.tx_freq_min = {\n";
        for (int radioIndex = 0; radioIndex < LGW_RF_CHAIN_NB; radioIndex++) {
            retVal << "\t\t\t\t" << value.tx_freq_min[radioIndex];
            if (radioIndex < LGW_RF_CHAIN_NB - 1)
                retVal << ",";
            retVal << "\n";
        }
        retVal << "\t\t\t},\n\t\t\t.tx_freq_max = {\n";
        for (int radioIndex = 0; radioIndex < LGW_RF_CHAIN_NB; radioIndex++) {
            retVal << "\t\t\t\t" << value.tx_freq_max[radioIndex];
            if (radioIndex < LGW_RF_CHAIN_NB - 1)
                retVal << ",";
            retVal << "\n";
        }
        retVal << "\t\t\t},\n\t\t\t.txLut = {\n";
        for (int radioIndex = 0; radioIndex < LGW_RF_CHAIN_NB; radioIndex++) {
            retVal << "\t\t\t\t{\n";
            if (value.txLut[radioIndex].size) {
                retVal << "\t\t\t\t\t.lut = {\n"
                    "\t\t\t\t\t\t// TX LUT radio " << radioIndex << ", count: " << (int) value.txLut[radioIndex].size << "\n";
                bool isFirst = true;
                for (int i = 0; i < value.txLut[radioIndex].size; i++) {
                    if (isFirst)
                        isFirst = false;
                    else
                        retVal << ",\n";
                    retVal << "\t\t\t\t\t\t{\n"
                        "\t\t\t\t\t\t\t\t.rf_power = " << (int) value.txLut[radioIndex].lut[i].rf_power << ",\n"
                        "\t\t\t\t\t\t\t\t.dig_gain = " << (int) value.txLut[radioIndex].lut[i].dig_gain << ",\n"
                        "\t\t\t\t\t\t\t\t.pa_gain = " << (int) value.txLut[radioIndex].lut[i].pa_gain << ",\n"
                        "\t\t\t\t\t\t\t\t.dac_gain = " << (int) value.txLut[radioIndex].lut[i].dac_gain << ",\n"
                        "\t\t\t\t\t\t\t\t.mix_gain = " << (int) value.txLut[radioIndex].lut[i].mix_gain << ",\n"
                        "\t\t\t\t\t\t\t\t.offset_i = " << (int) value.txLut[radioIndex].lut[i].offset_i << ",\n"
                        "\t\t\t\t\t\t\t\t.offset_q = " << (int) value.txLut[radioIndex].lut[i].offset_q << ",\n"
                        "\t\t\t\t\t\t\t\t.pwr_idx = " << (int) value.txLut[radioIndex].lut[i].pwr_idx << "\n"
                        "\t\t\t\t\t\t}";
                }
                retVal << "\n\t\t\t\t\t},\n";
            }
            retVal << "\t\t\t\t\t.size = " << (int) value.txLut[radioIndex].size;
            retVal << "\n\t\t\t\t}";
            if (radioIndex < LGW_RF_CHAIN_NB - 1)
                retVal << ",";
            retVal << "\n";
        }
        retVal << "\t\t\t},\n"
            "\t\t\t.ifConfs = {\n";
        for (unsigned char ch = 0; ch < LGW_MULTI_NB; ch++) {
            retVal
                << "\t\t\t\t{\n\t\t\t\t\t// chan_multiSF_" << (int) ch << "\n"
                "\t\t\t\t\t.enable = " << (value.ifConfs[ch].enable ? "true" : "false") << ",\n"
                "\t\t\t\t\t.rf_chain = " << (int) value.ifConfs[ch].rf_chain << ",\n"
                "\t\t\t\t\t.freq_hz = " << value.ifConfs[ch].freq_hz << "\n"
                "\t\t\t\t}";
            if (ch < LGW_MULTI_NB - 1)
                retVal << ",";
            retVal << "\n";
        }
        retVal << "\t\t\t},\n\t\t\t// Lora std \n\t\t\t.ifStdConf = {\n"
            "\t\t\t\t.enable = " << (value.ifStdConf.enable ? "true" : "false") << ",\n"
            "\t\t\t\t.rf_chain = " << (int) value.ifStdConf.rf_chain << ",\n"
            "\t\t\t\t.freq_hz = " << value.ifStdConf.freq_hz << ",\n"
            "\t\t\t\t.bandwidth = " << (int) value.ifStdConf.bandwidth << ",\n"
            "\t\t\t\t.datarate = " << value.ifStdConf.datarate << ",\n"
            "\t\t\t\t.implicit_hdr = " << (value.ifStdConf.implicit_hdr ? "true" : "false") << ",\n"
            "\t\t\t\t.implicit_payload_length = " << (int) value.ifStdConf.implicit_payload_length << ",\n"
            "\t\t\t\t.implicit_crc_en = " << (value.ifStdConf.implicit_crc_en ? "true" : "false") << ",\n"
            "\t\t\t\t.implicit_coderate = " << (int) value.ifStdConf.implicit_coderate << "\n"
            "\t\t\t},\n\t\t\t// FSK \n\t\t\t.ifFSKConf = {\n"
            "\t\t\t\t.enable = " << (value.ifFSKConf.enable ? "true" : "false") << ",\n"
            "\t\t\t\t.rf_chain = " << (int) value.ifFSKConf.rf_chain << ",\n"
            "\t\t\t\t.freq_hz = " << value.ifFSKConf.freq_hz << ",\n"
            "\t\t\t\t.bandwidth = " << (int) value.ifFSKConf.bandwidth << ",\n"
            "\t\t\t\t.datarate = " << value.ifFSKConf.datarate << "\n"

            "\t\t\t},\n\t\t\t.demodConf = {\n"
            "\t\t\t\t// chan_multiSF_All spreading_factor_enable bit field\n"
            "\t\t\t\t.multisf_datarate = 0x" << std::hex << (int) value.demodConf.multisf_datarate << std::dec
            << "\n\t\t\t}\n\t\t}";                                                                 // }
    } else {
        retVal << "\t{ // sx130x\n"
            "\t\t{ // boardConf\n"
            "\t\t\t" << (value.boardConf.lorawan_public ? "true" : "false") << ", // lorawan_public\n"
            "\t\t\t" << (int) value.boardConf.clksrc << ", // clksrc\n"
            "\t\t\t" << (value.boardConf.full_duplex ? "true" : "false") << ", // full_duplex\n"
            "\t\t\t(lgw_com_type_t) " << (int) value.boardConf.com_type << ", // com_type\n"
            "\t\t\t\"" << value.boardConf.com_path << "\" // com_path\n"
            "\t\t},\n"
            "\t\t" << (int) value.antennaGain << ", // antennaGain\n"
            "\t\t{ // tsConf\n"
            "\t\t\t" << (value.tsConf.enable ? "true" : "false") << ", // enable\n"
            "\t\t\t(lgw_ftime_mode_t) " << (int) value.tsConf.mode << " // mode\n"
            "\t\t},\n"
            "\t\t{ // rfConfs";
            for (int radioIndex = 0; radioIndex < LGW_RF_CHAIN_NB; radioIndex++) {
                retVal << "\n\t\t\t// Radio " << radioIndex << "\n\t\t\t{\n"
                "\t\t\t\t" << (value.rfConfs[radioIndex].enable ? "true" : "false") << ", // enable\n"
                "\t\t\t\t" << value.rfConfs[radioIndex].freq_hz << ", // freq_hz\n"
                "\t\t\t\t" << value.rfConfs[radioIndex].rssi_offset << ", // rssi_offset\n"
                "\t\t\t\t{ // rssi_tcomp\n"
                "\t\t\t\t\t" << value.rfConfs[radioIndex].rssi_tcomp.coeff_a << ", // coeff_a\n"
                "\t\t\t\t\t" << value.rfConfs[radioIndex].rssi_tcomp.coeff_b << ", // coeff_b\n"
                "\t\t\t\t\t" << value.rfConfs[radioIndex].rssi_tcomp.coeff_c << ", // coeff_c\n"
                "\t\t\t\t\t" << value.rfConfs[radioIndex].rssi_tcomp.coeff_d << ", // coeff_d\n"
                "\t\t\t\t\t" << value.rfConfs[radioIndex].rssi_tcomp.coeff_e << " // coeff_e\n"
                "\t\t\t\t},\n"
                "\t\t\t\t(lgw_radio_type_t) " << (int) value.rfConfs[radioIndex].type << ", // type\n"
                "\t\t\t\t" << (value.rfConfs[radioIndex].tx_enable ? "true" : "false") << ", // tx_enable\n"
                "\t\t\t\t" << (value.rfConfs[radioIndex].single_input_mode ? "true" : "false") << " // single_input_mode\n"
                "\t\t\t}";
            if (radioIndex < LGW_RF_CHAIN_NB - 1)
                retVal << ",";
        }
        retVal << "\n\t\t},\n\t\t{ // tx_freq_min\n ";
        for (int radioIndex = 0; radioIndex < LGW_RF_CHAIN_NB; radioIndex++) {
            retVal << "\t\t\t" << value.tx_freq_min[radioIndex];
            if (radioIndex < LGW_RF_CHAIN_NB - 1)
                retVal << ",";
            retVal << "\n";
        }
        retVal << "\t\t},\n\t\t{ // tx_freq_max\n";
        for (int radioIndex = 0; radioIndex < LGW_RF_CHAIN_NB; radioIndex++) {
            retVal << "\t\t\t" << value.tx_freq_max[radioIndex];
            if (radioIndex < LGW_RF_CHAIN_NB - 1)
                retVal << ",";
            retVal << "\n";
        }
        retVal << "\t\t},\n\t\t{ // txLut\n";
        for (int radioIndex = 0; radioIndex < LGW_RF_CHAIN_NB; radioIndex++) {
            retVal << "\t\t\t{\n";

            retVal << "\t\t\t\t{ // lut " << radioIndex <<"\n"
                      "\t\t\t\t\t// TX LUT radio " << radioIndex << ", count: " << (int) value.txLut[radioIndex].size << "\n";
            bool isFirst = true;
            for (int i = 0; i < value.txLut[radioIndex].size; i++) {
                if (isFirst)
                    isFirst = false;
                else
                    retVal << ",\n";
                retVal << "\t\t\t\t\t{\n"
                    "\t\t\t\t\t\t\t" << (int) value.txLut[radioIndex].lut[i].rf_power << ", // rf_power\n"
                    "\t\t\t\t\t\t\t" << (int) value.txLut[radioIndex].lut[i].dig_gain << ", // dig_gain\n"
                    "\t\t\t\t\t\t\t" << (int) value.txLut[radioIndex].lut[i].pa_gain << ", // pa_gain\n"
                    "\t\t\t\t\t\t\t" << (int) value.txLut[radioIndex].lut[i].dac_gain << ", // dac_gain\n"
                    "\t\t\t\t\t\t\t" << (int) value.txLut[radioIndex].lut[i].mix_gain << ", // mix_gain\n"
                    "\t\t\t\t\t\t\t" << (int) value.txLut[radioIndex].lut[i].offset_i << ", // offset_i\n"
                    "\t\t\t\t\t\t\t" << (int) value.txLut[radioIndex].lut[i].offset_q << ", // offset_q\n"
                    "\t\t\t\t\t\t\t" << (int) value.txLut[radioIndex].lut[i].pwr_idx << " // pwr_idx\n"
                    "\t\t\t\t\t}";
            }
            retVal << "\n\t\t\t\t},\n\t\t\t\t" << (int) value.txLut[radioIndex].size << " // size\n\t\t\t}";
            if (radioIndex < LGW_RF_CHAIN_NB - 1)
                retVal << ",";
            retVal << "\n";
        }
        retVal << "\t\t},\n"
            "\t\t{ // ifConfs\n";
        for (unsigned char ch = 0; ch < LGW_MULTI_NB; ch++) {
            retVal << "\t\t\t{\n\t\t\t\t// chan_multiSF_" << (int) ch << "\n"
            "\t\t\t\t" << (value.ifConfs[ch].enable ? "true" : "false") << ", // enable\n"
            "\t\t\t\t" << (int) value.ifConfs[ch].rf_chain << ", // rf_chain\n"
            "\t\t\t\t" << value.ifConfs[ch].freq_hz << " // freq_hz\n"
            "\t\t\t}";
            if (ch < LGW_MULTI_NB - 1)
                retVal << ",";
            retVal << "\n";
        }
        retVal << "\t\t},\n\t\t// Lora std \n\t\t{ // ifStdConf\n"
            "\t\t\t" << (value.ifStdConf.enable ? "true" : "false") << ", // enable\n"
            "\t\t\t" << (int) value.ifStdConf.rf_chain << ", // rf_chain\n"
            "\t\t\t" << value.ifStdConf.freq_hz << ", // freq_hz\n"
            "\t\t\t" << (int) value.ifStdConf.bandwidth << ", // bandwidth\n"
            "\t\t\t" << value.ifStdConf.datarate << ", // datarate\n"
            "\t\t\t" << (value.ifStdConf.implicit_hdr ? "true" : "false") << ", // implicit_hdr\n"
            "\t\t\t" << (int) value.ifStdConf.implicit_payload_length << ", // implicit_payload_length\n"
            "\t\t\t" << (value.ifStdConf.implicit_crc_en ? "true" : "false") << ", // implicit_crc_en\n"
            "\t\t\t" << (int) value.ifStdConf.implicit_coderate << " // implicit_coderate\n"
            "\t\t},\n\t\t// FSK \n\t\t{ // ifFSKConf\n"
            "\t\t\t" << (value.ifFSKConf.enable ? "true" : "false") << ", // enable\n"
            "\t\t\t" << (int) value.ifFSKConf.rf_chain << ", // rf_chain\n"
            "\t\t\t" << value.ifFSKConf.freq_hz << ", // freq_hz\n"
            "\t\t\t" << (int) value.ifFSKConf.bandwidth << ", // bandwidth\n"
            "\t\t\t" << value.ifFSKConf.datarate << " // datarate\n"
            "\t\t},\n\t\t{ // demodConf\n"
            "\t\t\t// chan_multiSF_All spreading_factor_enable bit field\n"
            "\t\t\t0x" << std::hex << (int) value.demodConf.multisf_datarate << std::dec
            << " // multisf_datarate\n\t\t}\n\t}";
    }
}

void GatewaySX130xConfig::toJSON(
    nlohmann::json &jsonValue
) const {
    nlohmann::json jSx1261;
    sx1261Config.toJSON(jSx1261);
    jsonValue["sx1261_conf"] = jSx1261;

    jsonValue["com_type"] = lgw_com_type_t2string(value.boardConf.com_type);
    jsonValue["com_path"] = value.boardConf.com_path;
    jsonValue["lorawan_public"] = value.boardConf.lorawan_public;
    jsonValue["clksrc"] = value.boardConf.clksrc;
    jsonValue["antenna_gain"] = value.antennaGain;
    jsonValue["full_duplex"] = value.boardConf.full_duplex;

    nlohmann::json fine_timestamp;
    fine_timestamp["enable"] = value.tsConf.enable;
    fine_timestamp["mode"] = lgw_ftime_mode_t2string(value.tsConf.mode);
    jsonValue["fine_timestamp"] = fine_timestamp;

    std::string ridx = "radio_0";
    for (int radioIndex = 0; radioIndex < LGW_RF_CHAIN_NB; radioIndex++) {
        nlohmann::json jRadio;
        jRadio["enable"] = value.rfConfs[radioIndex].enable;
        jRadio["type"] = lgw_radio_type_t2string(value.rfConfs[radioIndex].type);
        jRadio["freq"] = value.rfConfs[radioIndex].freq_hz;
        jRadio["rssi_offset"] = value.rfConfs[radioIndex].rssi_offset;
        jRadio["rssi_offset"] = value.rfConfs[radioIndex].rssi_offset;

        nlohmann::json jRadioRssiTcomp;
        jRadioRssiTcomp["coeff_a"] = value.rfConfs[radioIndex].rssi_tcomp.coeff_a;
        jRadioRssiTcomp["coeff_b"] = value.rfConfs[radioIndex].rssi_tcomp.coeff_b;
        jRadioRssiTcomp["coeff_c"] = value.rfConfs[radioIndex].rssi_tcomp.coeff_c;
        jRadioRssiTcomp["coeff_d"] = value.rfConfs[radioIndex].rssi_tcomp.coeff_d;
        jRadioRssiTcomp["coeff_e"] = value.rfConfs[radioIndex].rssi_tcomp.coeff_e;
        jRadio["rssi_tcomp"] = jRadioRssiTcomp;

        jRadio["tx_enable"] = value.rfConfs[radioIndex].tx_enable;
        jRadio["single_input_mode"] = value.rfConfs[radioIndex].single_input_mode;
        jRadio["tx_freq_min"] = value.tx_freq_min[radioIndex];
        jRadio["tx_freq_max"] = value.tx_freq_max[radioIndex];

        nlohmann::json jRadioTxGainLuts;
        for (int i = 0; i < value.txLut[radioIndex].size; i++) {
            nlohmann::json jRadioTxGainLut;
            jRadioTxGainLut["rf_power"] = value.txLut[radioIndex].lut[i].rf_power;
            jRadioTxGainLut["pa_gain"] = value.txLut[radioIndex].lut[i].pa_gain;
            jRadioTxGainLut["pwr_idx"] = value.txLut[radioIndex].lut[i].pwr_idx;
            jRadioTxGainLut["dig_gain"] = value.txLut[radioIndex].lut[i].dig_gain;
            jRadioTxGainLut["dac_gain"] = value.txLut[radioIndex].lut[i].dac_gain;
            jRadioTxGainLut["mix_gain"] = value.txLut[radioIndex].lut[i].mix_gain;
            jRadioTxGainLuts.push_back(jRadioTxGainLut);
        }
        jRadio["tx_gain_lut"] = jRadioTxGainLuts;
        ridx[6] = '0' + radioIndex;
        jsonValue[ridx] = jRadio;
    }

    nlohmann::json jChanMultiSFAll;
    nlohmann::json jSpreadingFactorEnables;
    for (int n = 0; n < 8; n++) {
        if (value.demodConf.multisf_datarate & (1 << n)) {
            jSpreadingFactorEnables.push_back(n + 5);
        }
    }
    jChanMultiSFAll["spreading_factor_enable"] = jSpreadingFactorEnables;

    std::string cmsfn = "chan_multiSF_0";
    for (unsigned char ch = 0; ch < LGW_MULTI_NB; ch++) {
        nlohmann::json jChannelSF;
        jChannelSF["enable"] = value.ifConfs[ch].enable;
        jChannelSF["radio"] = value.ifConfs[ch].rf_chain;
        jChannelSF["if"] = value.ifConfs[ch].freq_hz;
        cmsfn[13] = ch + '0';
        jsonValue[cmsfn] = jChannelSF;
    }
    jsonValue["chan_multiSF_All"] = jChanMultiSFAll;

    // Lora std
    nlohmann::json jChanLoraStd;
    jChanLoraStd["enable"] = value.ifStdConf.enable;
    jChanLoraStd["radio"] = value.ifStdConf.rf_chain;
    jChanLoraStd["if"] = value.ifStdConf.freq_hz;
    jChanLoraStd["bandwidth"] = bandwidthIndex2hz(value.ifStdConf.bandwidth);
    jChanLoraStd["spread_factor"] = value.ifStdConf.datarate;
    jChanLoraStd["implicit_hdr"] = value.ifStdConf.implicit_hdr;
    jChanLoraStd["implicit_payload_length"] = value.ifStdConf.implicit_payload_length;
    jChanLoraStd["implicit_crc_en"] = value.ifStdConf.implicit_crc_en;
    jChanLoraStd["implicit_coderate"] = value.ifStdConf.implicit_coderate;
    jsonValue["chan_Lora_std"] = jChanLoraStd;

    nlohmann::json jChannelFSK;
    jChannelFSK["enable"] = value.ifFSKConf.enable;
    jChannelFSK["radio"] = value.ifFSKConf.rf_chain;
    jChannelFSK["if"] = value.ifFSKConf.freq_hz;
    jChannelFSK["bandwidth"] = bandwidthIndex2hz(value.ifFSKConf.bandwidth);
    jChannelFSK["datarate"] = value.ifFSKConf.datarate;
    jsonValue["chan_FSK"] = jChannelFSK;
}

/*
    "gateway_conf": {
        "gateway_ID": "AA555A0000000000",
        "server_address": "84.237.104.128",
        "serv_port_up": 5000,
        "serv_port_down": 5000,
        "keepalive_interval": 10,
        "stat_interval": 30,
        "push_timeout_ms": 100,
        "forward_crc_valid": true,
        "forward_crc_error": false,
        "forward_crc_disabled": false,
        "gps_tty_path": "/dev/ttyS0",
        "ref_latitude": 0.0,
        "ref_longitude": 0.0,
        "ref_altitude": 0,
        "beaconPeriod": 0,
        "beaconFreqHz": 869525000,
        "beaconDataRate": 9,
        "beaconBandwidthHz": 125000,
        "beaconPower": 14,
        "beaconInfoDesc": 0
    }
*/

GatewayGatewayConfig::GatewayGatewayConfig()
{
    reset();
}

void GatewayGatewayConfig::reset()
{
    value.gatewayId = 0;
    value.serverPortUp = 0;
    value.serverPortDown = 0;
    value.keepaliveInterval = 0;
    value.statInterval = DEFAULT_KEEPALIVE;
    value.forwardCRCValid = false;
    value.forwardCRCError = false;
    value.forwardCRCDisabled = false;
    value.gpsEnabled = false;
    value.fakeGPS = false;
    value.beaconPeriod = 0;
    value.beaconFreqHz = DEFAULT_BEACON_FREQ_HZ;
    value.beaconFreqNb = DEFAULT_BEACON_FREQ_NB;
    value.beaconFreqStep = 0;
    value.beaconDataRate = DEFAULT_BEACON_DATARATE;
    value.beaconBandwidthHz = DEFAULT_BEACON_BW_HZ;
    value.beaconPower = DEFAULT_BEACON_POWER;
    value.beaconInfoDesc = 0;
    value.autoQuitThreshold = 0;

    value.pushTimeoutMs.tv_sec = 0;
    value.pushTimeoutMs.tv_usec = 0;
    value.refGeoCoordinates.lat = 0.0;
    value.refGeoCoordinates.lon = 0.0;
    value.refGeoCoordinates.alt = 0;
}

int GatewayGatewayConfig::parse(nlohmann::json &jsonValue)
{
    reset();
    auto jGatewayId = jsonValue.find("gateway_ID");
    if (jGatewayId != jsonValue.end()) {
        if (jGatewayId->is_string()) {
            std::string s = *jGatewayId;
            value.gatewayId = std::stoull(s.c_str(), nullptr, 16);
        }
    }
    auto jServerAddress = jsonValue.find("server_address");
    if (jServerAddress != jsonValue.end()) {
        if (jServerAddress->is_string()) {
            serverAddr = *jServerAddress;
        }
    }
    auto jServerPortUp = jsonValue.find("serv_port_up");
    if (jServerPortUp != jsonValue.end()) {
        if (jServerPortUp->is_number_unsigned()) {
            value.serverPortUp = *jServerPortUp;
        }
    }
    auto jServerPortDown = jsonValue.find("serv_port_down");
    if (jServerPortDown != jsonValue.end()) {
        if (jServerPortDown->is_number_unsigned()) {
            value.serverPortDown = *jServerPortDown;
        }
    }
    value.keepaliveInterval = DEFAULT_KEEPALIVE;
    auto jKeepaliveInterval = jsonValue.find("keepalive_interval");
    if (jKeepaliveInterval != jsonValue.end()) {
        if (jKeepaliveInterval->is_number_integer()) {
            value.keepaliveInterval = *jKeepaliveInterval;
        }
    }
    auto jStatInterval = jsonValue.find("stat_interval");
    if (jStatInterval != jsonValue.end()) {
        if (jStatInterval->is_number_integer()) {
            value.statInterval = *jStatInterval;
        }
    }
    auto jPushTimeoutMs = jsonValue.find("push_timeout_ms");
    if (jPushTimeoutMs != jsonValue.end()) {
        if (jPushTimeoutMs->is_number_integer()) {
            int m = *jPushTimeoutMs;
            value.pushTimeoutMs.tv_sec = 0;
            value.pushTimeoutMs.tv_usec = m * 500;
        }
    }
    auto jForwardCRCValid = jsonValue.find("forward_crc_valid");
    if (jForwardCRCValid != jsonValue.end()) {
        if (jForwardCRCValid->is_boolean()) {
            value.forwardCRCValid = *jForwardCRCValid;
        }
    }
    auto jForwardCRCError = jsonValue.find("forward_crc_error");
    if (jForwardCRCError != jsonValue.end()) {
        if (jForwardCRCError->is_boolean()) {
            value.forwardCRCError = *jForwardCRCError;
        }
    }
    auto jForwardCRCDisabled = jsonValue.find("forward_crc_disabled");
    if (jForwardCRCDisabled != jsonValue.end()) {
        if (jForwardCRCDisabled->is_boolean()) {
            value.forwardCRCDisabled = *jForwardCRCDisabled;
        }
    }
    auto jGpsTTYPath = jsonValue.find("gps_tty_path");
    if (jGpsTTYPath != jsonValue.end()) {
        if (jGpsTTYPath->is_string()) {
            gpsTtyPath = *jGpsTTYPath;
            value.gpsEnabled = !gpsTtyPath.empty();
        }
    }
    auto jRefLat = jsonValue.find("ref_latitude");
    if (jRefLat != jsonValue.end()) {
        if (jRefLat->is_number()) {
            value.refGeoCoordinates.lat = *jRefLat;
        }
    }
    auto jRefLon = jsonValue.find("ref_longitude");
    if (jRefLon != jsonValue.end()) {
        if (jRefLon->is_number()) {
            value.refGeoCoordinates.lon = *jRefLon;
        }
    }
    auto jRefAlt = jsonValue.find("ref_altitude");
    if (jRefAlt != jsonValue.end()) {
        if (jRefAlt->is_number()) {
            value.refGeoCoordinates.alt = *jRefAlt;
        }
    }
    auto jFakeGPS = jsonValue.find("fake_gps");
    if (jFakeGPS != jsonValue.end()) {
        if (jFakeGPS->is_boolean()) {
            value.fakeGPS = *jFakeGPS;
        }
    }
    auto jBeaconPeriod = jsonValue.find("beacon_period");
    if (jBeaconPeriod != jsonValue.end()) {
        if (jBeaconPeriod->is_number_integer()) {
            value.beaconPeriod = *jBeaconPeriod;
        }
    }
    value.beaconFreqHz = DEFAULT_BEACON_FREQ_HZ;
    auto jBeaconFreq = jsonValue.find("beacon_freq_hz");
    if (jBeaconFreq != jsonValue.end()) {
        if (jBeaconFreq->is_number_integer()) {
            value.beaconFreqHz = *jBeaconFreq;
        }
    }
    value.beaconFreqNb = DEFAULT_BEACON_FREQ_NB;
    auto jBeaconFreqNb = jsonValue.find("beacon_freq_nb");
    if (jBeaconFreqNb != jsonValue.end()) {
        if (jBeaconFreqNb->is_number_integer()) {
            value.beaconFreqNb = *jBeaconFreqNb;
        }
    }
    auto jBeaconFreqStep = jsonValue.find("beacon_freq_step");
    if (jBeaconFreqStep != jsonValue.end()) {
        if (jBeaconFreqStep->is_number_integer()) {
            value.beaconFreqStep = *jBeaconFreqStep;
        }
    }
    value.beaconDataRate = DEFAULT_BEACON_DATARATE;
    auto jBeaconDR = jsonValue.find("beacon_datarate");
    if (jBeaconDR != jsonValue.end()) {
        if (jBeaconDR->is_number_integer()) {
            value.beaconDataRate = *jBeaconDR;
        }
    }
    value.beaconBandwidthHz = DEFAULT_BEACON_BW_HZ;
    auto jBeaconBandwidthHz = jsonValue.find("beacon_bw_hz");
    if (jBeaconBandwidthHz != jsonValue.end()) {
        if (jBeaconBandwidthHz->is_number_integer()) {
            value.beaconBandwidthHz = *jBeaconBandwidthHz;
        }
    }
    value.beaconPower = DEFAULT_BEACON_POWER;
    auto jBeaconPower = jsonValue.find("beacon_power");
    if (jBeaconPower != jsonValue.end()) {
        if (jBeaconPower->is_number_integer()) {
            value.beaconPower = *jBeaconPower;
        }
    }
    auto jBeaconInfoDesc = jsonValue.find("beacon_infodesc");
    if (jBeaconInfoDesc != jsonValue.end()) {
        if (jBeaconInfoDesc->is_number_integer()) {
            value.beaconInfoDesc = *jBeaconInfoDesc;
        }
    }
    auto jAutoQuitThreshold = jsonValue.find("autoquit_threshold");
    if (jAutoQuitThreshold != jsonValue.end()) {
        if (jAutoQuitThreshold->is_number_integer()) {
            value.autoQuitThreshold = *jAutoQuitThreshold;
        }
    }
    return CODE_OK;
}

void GatewayGatewayConfig::toHeader(
    std::ostream &retVal,
    const std::string &name,
    bool cpp20
) const
{
    if (cpp20) {
        retVal <<
            "\t.gateway = {\n"
            "\t\t.gatewayId = 0x" << std::hex << value.gatewayId << std::dec << ",\n"
            "\t\t.serverPortUp = " << value.serverPortUp << ",\n"
            "\t\t.serverPortDown = " << value.serverPortDown << ",\n"
            "\t\t.keepaliveInterval = " << value.keepaliveInterval << ",\n"
            "\t\t.statInterval = " << value.statInterval << ",\n"
            "\t\t.pushTimeoutMs = {\n\t\t\t.tv_sec = " << value.pushTimeoutMs.tv_sec << ",\n"
            "\t\t\t.tv_usec = " << value.pushTimeoutMs.tv_usec << "\n\t\t},\n"
    
            "\t\t.forwardCRCValid = " << (value.forwardCRCValid ? "true" : "false") << ",\n"
            "\t\t.forwardCRCError = " << (value.forwardCRCError ? "true" : "false") << ",\n"
            "\t\t.forwardCRCDisabled = " << (value.forwardCRCDisabled ? "true" : "false") << ",\n"
            "\t\t.gpsEnabled = " << (value.gpsEnabled ? "true" : "false") << ",\n"
            "\t\t.refGeoCoordinates = {\n"
            "\t\t\t.lat = " << value.refGeoCoordinates.lat << ",\n"
            "\t\t\t.lon = " << value.refGeoCoordinates.lon << ",\n"
            "\t\t\t.alt = " << value.refGeoCoordinates.alt << "\n"
            "\t\t},\n"
            "\t\t.fakeGPS = " << (value.fakeGPS ? "true" : "false") << ",\n"
            "\t\t.beaconPeriod = " << value.beaconPeriod << ",\n"
            "\t\t.beaconFreqHz = " << value.beaconFreqHz << ",\n"
            "\t\t.beaconFreqNb = " << (int) value.beaconFreqNb << ",\n"
            "\t\t.beaconFreqStep = " << value.beaconFreqStep << ",\n"
            "\t\t.beaconDataRate = " << (int) value.beaconDataRate << ",\n"
            "\t\t.beaconBandwidthHz = " << value.beaconBandwidthHz << ",\n"
            "\t\t.beaconInfoDesc = " << (int) value.beaconInfoDesc << ",\n"
            "\t\t.autoQuitThreshold = " << value.autoQuitThreshold << "\n"
            "\t}";
    } else {
        retVal <<
            "\t{ // gateway\n"
            "\t\t0x" << std::hex << value.gatewayId << std::dec << ", // gatewayId\n"
            "\t\t" << value.serverPortUp << ", // serverPortUp\n"
            "\t\t" << value.serverPortDown << ", // serverPortDown\n"
            "\t\t" << value.keepaliveInterval << ", // keepaliveInterval\n"
            "\t\t" << value.statInterval << ", // statInterval\n"
            "\t\t{ // pushTimeoutMs\n\t\t\t" << value.pushTimeoutMs.tv_sec << ", // tv_sec\n"
            "\t\t\t" << value.pushTimeoutMs.tv_usec << " // tv_usec\n\t\t},\n"

            "\t\t" << (value.forwardCRCValid ? "true" : "false") << ", // forwardCRCValid\n"
            "\t\t" << (value.forwardCRCError ? "true" : "false") << ", // forwardCRCError\n"
            "\t\t" << (value.forwardCRCDisabled ? "true" : "false") << ", // forwardCRCDisabled\n"
            "\t\t" << (value.gpsEnabled ? "true" : "false") << ", // gpsEnabled\n"
            "\t\t{ // refGeoCoordinates\n"
            "\t\t\t" << value.refGeoCoordinates.lat << ", // lat\n"
            "\t\t\t" << value.refGeoCoordinates.lon << ", // lon\n"
            "\t\t\t" << value.refGeoCoordinates.alt << " // alt\n"
            "\t\t},\n"
            "\t\t" << (value.fakeGPS ? "true" : "false") << ", // fakeGPS\n"
            "\t\t" << value.beaconPeriod << ", // beaconPeriod\n"
            "\t\t" << value.beaconFreqHz << ", // beaconFreqHz\n"
            "\t\t" << (int) value.beaconFreqNb << ", // beaconFreqNb\n"
            "\t\t" << value.beaconFreqStep << ", // beaconFreqStep\n"
            "\t\t" << (int) value.beaconDataRate << ", // beaconDataRate\n"
            "\t\t" << value.beaconBandwidthHz << ", // beaconBandwidthHz\n"
            "\t\t" << (int) value.beaconInfoDesc << ", // beaconInfoDesc\n"
            "\t\t" << value.autoQuitThreshold << " // autoQuitThreshold\n"
            "\t}";
    }
}

void GatewayGatewayConfig::toJSON(
    nlohmann::json &jsonValue
) const
{
    std::stringstream ssGatewayId;
    ssGatewayId << std::hex << std::setw(16) << std::setfill('0') << value.gatewayId << std::dec;
    jsonValue["gateway_ID"] = ssGatewayId.str();
    jsonValue["server_address"] = serverAddr;
    jsonValue["serv_port_up"] = value.serverPortUp;
    jsonValue["serv_port_down"] = value.serverPortDown;
    jsonValue["keepalive_interval"] = value.keepaliveInterval;
    jsonValue["stat_interval"] = value.statInterval;
    jsonValue["push_timeout_ms"] = value.pushTimeoutMs.tv_usec / 500;
    jsonValue["forward_crc_valid"] = value.forwardCRCValid;
    jsonValue["forward_crc_error"] = value.forwardCRCError;
    jsonValue["forward_crc_disabled"] = value.forwardCRCDisabled;
    jsonValue["gps_tty_path"] = gpsTtyPath;
    jsonValue["ref_latitude"] = value.refGeoCoordinates.lat;
    jsonValue["ref_longitude"] = value.refGeoCoordinates.lon;
    jsonValue["ref_altitude"] = value.refGeoCoordinates.alt;
    jsonValue["fake_gps"] = value.fakeGPS;
    jsonValue["beacon_period"] = value.beaconPeriod;
    jsonValue["beacon_freq_hz"] = value.beaconFreqHz;
    jsonValue["beacon_freq_nb"] = value.beaconFreqNb;
    jsonValue["beacon_freq_step"] = value.beaconFreqStep;
    jsonValue["beacon_datarate"] = value.beaconDataRate;
    jsonValue["beacon_bw_hz"] = value.beaconBandwidthHz;
    jsonValue["beacon_power"] = value.beaconPower;
    jsonValue["beacon_infodesc"] = value.beaconInfoDesc;
    jsonValue["autoquit_threshold"] = value.autoQuitThreshold;
}

bool GatewayGatewayConfig::operator==(const GatewayGatewayConfig &b) const
{
    return value.gatewayId == b.value.gatewayId
        && serverAddr == b.serverAddr
        && value.serverPortUp == b.value.serverPortUp
        && value.serverPortDown == b.value.serverPortDown
        && value.keepaliveInterval == b.value.keepaliveInterval
        && value.statInterval == b.value.statInterval
        && (memcmp(&value.pushTimeoutMs, &b.value.pushTimeoutMs, sizeof(timeval)) == 0)
        && value.forwardCRCValid == b.value.forwardCRCValid
        && value.forwardCRCError == b.value.forwardCRCError
        && value.forwardCRCDisabled == b.value.forwardCRCDisabled
        && gpsTtyPath == b.gpsTtyPath
        && (fabs(value.refGeoCoordinates.lat - b.value.refGeoCoordinates.lat) < 0.00001)
        && (fabs(value.refGeoCoordinates.lon - b.value.refGeoCoordinates.lon) < 0.00001)
        && (abs(value.refGeoCoordinates.alt - b.value.refGeoCoordinates.alt)  == 0)
        && value.fakeGPS == b.value.fakeGPS
        && value.beaconPeriod == b.value.beaconPeriod
        && value.beaconFreqHz == b.value.beaconFreqHz
        && value.beaconFreqNb == b.value.beaconFreqNb
        && value.beaconFreqStep == b.value.beaconFreqStep
        && value.beaconDataRate == b.value.beaconDataRate
        && value.beaconBandwidthHz == b.value.beaconBandwidthHz
        && value.beaconPower == b.value.beaconPower
        && value.beaconInfoDesc == b.value.beaconInfoDesc
        && value.autoQuitThreshold == b.value.autoQuitThreshold;
}

/**
   "ref_payload":[
        {"id": "0xCAFE1234"},
        {"id": "0xCAFE2345"}
    ],
    "log_file": "loragw_hal.log"
*/
GatewayDebugConfig::GatewayDebugConfig()
{
    reset();
}

void GatewayDebugConfig::reset()
{
    memset(&value, 0, sizeof(value));
}

int GatewayDebugConfig::parse(nlohmann::json &jsonValue)
{
    reset();
    auto jLogFileName = jsonValue.find("log_file");
    if (jLogFileName != jsonValue.end()) {
        if (jLogFileName->is_string()) {
            std::string s = *jLogFileName;
            size_t sz = s.size();
            if (sz < 128) {
                strncpy(&value.log_file_name[0], s.c_str(), 128);
                value.log_file_name[sz] = 0;
            }
        }
    }
    auto jRefPayloads = jsonValue.find("ref_payload");
    if (jRefPayloads != jsonValue.end()) {
        if (jRefPayloads->is_array()) {
            value.nb_ref_payload = jRefPayloads->size();
            if (value.nb_ref_payload > 16)
                value.nb_ref_payload = 16;
            for (int i = 0; i < value.nb_ref_payload; i++) {
                nlohmann::json jrp = jRefPayloads->at(i);
                if (jrp.is_object()) {
                    auto jrpId = jrp.find("id");
                    if (jrpId != jrp.end()) {
                        if (jrpId->is_string()) {
                            std::string s = *jrpId;
                            value.ref_payload[i].id = std::stoull(s.c_str(), nullptr, 0);
                        }
                    }
                }
            }
        }
    }
    return CODE_OK;
}

void GatewayDebugConfig::toHeader(
    std::ostream &retVal,
    const std::string &name,
    bool cpp20
) const
{
    if (cpp20) {
        retVal << "\t// Debug nb_ref_payload, count: " << (int) value.nb_ref_payload << "\n"
            "\t.debug = {\n"
            "\t\t.nb_ref_payload = "
            << (int) value.nb_ref_payload << ",\n";
        if (value.nb_ref_payload) {
            // identifiers
            retVal << "\t\t.ref_payload = {\n";
            bool isFirst = true;
            for (int i = 0; i < value.nb_ref_payload; i++) {
                if (isFirst)
                    isFirst = false;
                else
                    retVal << ",\n";
                retVal << "\t\t\t{\n\t\t\t\t.id = 0x" << std::hex << value.ref_payload[i].id << "\n\t\t\t}" << std::dec;
            }
            retVal << "\n\t\t},\n";
        }
        // log file name
        retVal << "\t\t.log_file_name = \"" << value.log_file_name << "\"\n";
        retVal << "\t}";
    } else {
        retVal << "\t// Debug nb_ref_payload, count: " << (int) value.nb_ref_payload << "\n"
            "\t{ // debug\n"
            "\t\t" << (int) value.nb_ref_payload << ", // nb_ref_payload\n";
        // identifiers
        retVal << "\t\t{ // ref_payload\n";
        bool isFirst = true;
        for (int i = 0; i < value.nb_ref_payload; i++) {
            if (isFirst)
                isFirst = false;
            else
                retVal << ",\n";
            retVal << "\t\t\t{\n\t\t\t\t0x" << std::hex << value.ref_payload[i].id << " // id\n\t\t\t}" << std::dec;
        }
        retVal << "\n\t\t},\n";

        // log file name
        retVal << "\t\t\"" << value.log_file_name << "\" // log_file_name\n";
        retVal << "\t}";
    }
}

void GatewayDebugConfig::toJSON(
    nlohmann::json &jsonValue
) const
{
    nlohmann::json jrefPayloads;
    // identifiers
    for (int i = 0; i < value.nb_ref_payload; i++) {
        nlohmann::json jrp;
        std::stringstream ss;
        ss << "0x" << std::hex << std::setfill('0') << std::setw(8) << value.ref_payload[i].id << std::dec;
        jrp["id"] = ss.str();
        jrefPayloads.push_back(jrp);
    }
    jsonValue["ref_payload"] = jrefPayloads;

    // log file name
    jsonValue["log_file"] = value.log_file_name;
}

bool GatewayDebugConfig::operator==(const GatewayDebugConfig &b) const
{
    return memcmp(&value, &b.value, sizeof(struct lgw_conf_debug_s)) == 0;
}

GatewayConfigFileJson::GatewayConfigFileJson()
{

}

GatewayConfigFileJson::~GatewayConfigFileJson()
{

}

void GatewayConfigFileJson::reset()
{
    sx130xConf.reset();
    gatewayConf.reset();
    debugConf.reset();
}

int GatewayConfigFileJson::parse(
    nlohmann::json &jsonValue
)
{
    int r = CODE_OK;
    auto jSx130x = jsonValue.find("SX130x_conf");
    if (jSx130x != jsonValue.end()) {
        r = sx130xConf.parse(*jSx130x);
        if (r)
            return r;
    }
    auto jGateway = jsonValue.find("gateway_conf");
    if (jGateway != jsonValue.end()) {
        r = gatewayConf.parse(*jGateway);
        if (r)
            return r;
    }
    auto jDebug = jsonValue.find("debug_conf");
    if (jDebug != jsonValue.end()) {
        r = debugConf.parse(*jDebug);
    }
    return r;
}

void GatewayConfigFileJson::toHeader(
    std::ostream &retVal,
    const std::string &name,
    bool cpp20
) const
{
    retVal << "{\n";
    sx130xConf.sx1261Config.toHeader(retVal, name + ".sx1261", cpp20);
    retVal << ",\n";
    sx130xConf.toHeader(retVal, name + ".sx130x", cpp20);
    retVal << ",\n";
    gatewayConf.toHeader(retVal, name + ".gateway", cpp20);
    retVal << ",\n";
    std::string niceName(name);
    std::replace(niceName.begin(), niceName.end(), '_', '-');
    std::transform(niceName.begin(), niceName.end(), niceName.begin(), [](char x) {
        if (::isalpha(x))
            return x ^ 32;
        else
            return (int) x;
    });
    debugConf.toHeader(retVal, name + ".debug", cpp20);
    if (cpp20)
        retVal << ",\n\t.serverAddr = \"" << gatewayConf.serverAddr << "\",\n"
            "\t.gpsTtyPath = \"" << gatewayConf.gpsTtyPath << "\",\n"
            "\t.name = \"" << niceName << "\",\n}";
    else
        retVal << ",\n\t\"" << gatewayConf.serverAddr << "\", // serverAddr\n"
        "\t\"" << gatewayConf.gpsTtyPath << "\", // gpsTtyPath\n"
        "\t\"" << niceName << "\" // name\n}";
}

void GatewayConfigFileJson::toJSON(
    nlohmann::json &jsonValue
) const {
    nlohmann::json jSx130x;
    sx130xConf.toJSON(jSx130x);
    jsonValue["SX130x_conf"] = jSx130x;
    nlohmann::json jGateway;
    gatewayConf.toJSON(jGateway);
    jsonValue["gateway_conf"] = jGateway;
    nlohmann::json jDebug;
    debugConf.toJSON(jDebug);
    jsonValue["debug_conf"] = jDebug;
}

bool GatewayConfigFileJson::operator==(
    const GatewayConfigFileJson &b
) const
{
    return (sx130xConf == b.sx130xConf)
        && (gatewayConf == b.gatewayConf)
        && (debugConf == b.debugConf);
}

std::string GatewayConfigFileJson::toString() const {
    nlohmann::json doc;
    toJSON(doc);
    return doc.dump();
}
