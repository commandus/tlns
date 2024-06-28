#include <sstream>
#include <cstdarg>
#include <iostream>
#include <iomanip>

#include "lorawan/regional-parameters/regional-parameter-channel-plan.h"
#include "lorawan/lorawan-string.h"

static std::string STR_TRUE("true");
static std::string STR_FALSE("false");

#define STR_TRUE_FALSE STR_TRUE : STR_FALSE

DataRate::DataRate()
    : value { .uplink = true, .downlink = true, .modulation = MODULATION_LORA,
      .bandwidth = BANDWIDTH_INDEX_125KHZ, .spreadingFactor = DRLORA_SF11, .bps = 00 }
{

}

DataRate::DataRate(
    const DataRate &val
)
    : value { .uplink = val.value.uplink, .downlink = val.value.downlink, .modulation = val.value.modulation,
        .bandwidth = val.value.bandwidth, .spreadingFactor = val.value.spreadingFactor, .bps = val.value.bps }
{

}

DataRate::DataRate(
    const DATA_RATE &val
)
    : value { .uplink = val.uplink, .downlink = val.downlink, .modulation = val.modulation,
        .bandwidth = val.bandwidth, .spreadingFactor = val.spreadingFactor, .bps = val.bps }
{

}

DataRate::DataRate(
    BANDWIDTH aBandwidth,
    SPREADING_FACTOR aSpreadingFactor
)
    : value { .uplink = true, .downlink = true, .modulation = MODULATION_LORA,
        .bandwidth = aBandwidth, .spreadingFactor = aSpreadingFactor, .bps = 0 }
{

}

DataRate::DataRate(uint32_t aBps)
    : value { .uplink = true, .downlink = true, .modulation = MODULATION_FSK,
      .bandwidth = BANDWIDTH_INDEX_7KHZ, .spreadingFactor = DRLORA_SF5, .bps = aBps }
{

}

void DataRate::setLora(
    BANDWIDTH aBandwidth,
    SPREADING_FACTOR aSpreadingFactor
)
{
    value.uplink = true;
    value.downlink = true;
    value.modulation = MODULATION_LORA;
    value.bandwidth = aBandwidth;
    value.spreadingFactor = aSpreadingFactor;
    value.bps = 0;
}

void DataRate::setFSK(uint32_t aBps)
{
    value.uplink = true;
    value.downlink = true;
    value.modulation = MODULATION_FSK;
    value.bandwidth = BANDWIDTH_INDEX_7KHZ;
    value.spreadingFactor = DRLORA_SF5;
    value.bps = aBps;
}

std::string DataRate::toString() const
{
    std::stringstream ss;
    // std::boolalpha
    ss << "{\"uplink\": " << (value.uplink ? STR_TRUE_FALSE)
        << ", \"downlink\": " << (value.downlink ? STR_TRUE_FALSE)
        << ", \"modulation\": \"" << MODULATION2String(value.modulation)
        << "\", \"bandwidth\": " <<  BANDWIDTH2String(value.bandwidth)
        << ", \"spreadingFactor\": " << value.spreadingFactor
        << ", \"bps\": " <<  value.bps
        << "}";
    return ss.str();
}

Channel::Channel()
    : value { .frequency = 0, .minDR = 0, .maxDR = 0,
        .enabled = true, .custom = false }
{

}

Channel::Channel(const Channel &val)
    : value { .frequency = val.value.frequency, .minDR = val.value.minDR,
          .maxDR = val.value.maxDR, .enabled = val.value.enabled,
          .custom = val.value.custom }
{

}

Channel::Channel(
    const RADIO_CHANNEL &value
)
    : value { .frequency = value.frequency, .minDR = value.minDR,
        .maxDR = value.maxDR, .enabled = value.enabled,
        .custom = value.custom }
{

}

std::string Channel::toString() const
{
    std::stringstream ss;
    ss << "{\"frequency\": " << value.frequency  // frequency in Hz
       << ", \"minDR\": " << value.minDR
       << ", \"maxDR\": " << value.maxDR
       << ", \"enabled\": " << (value.enabled ? STR_TRUE_FALSE)
       << ", \"custom\": " << (value.custom ? STR_TRUE_FALSE)
       << "}";
    return ss.str();
}

void Channel::setValue(int aFrequency, int aMinDR, int aMaxDR, bool aEnabled, bool aCustom)
{
    value.frequency = aFrequency;
    value.minDR = aMinDR;
    value.maxDR = aMaxDR;
    value.enabled = aEnabled;
    value.custom = aCustom;
}

BandDefaults::BandDefaults()
    : value { .RX2Frequency = 0, .RX2DataRate = 0, .ReceiveDelay1 = 0,
        .ReceiveDelay2 = 0, .JoinAcceptDelay1 = 0, .JoinAcceptDelay2 = 0 }
{

}

BandDefaults::BandDefaults(const BandDefaults& val)
    : value { .RX2Frequency = val.value.RX2Frequency, .RX2DataRate = val.value.RX2DataRate,
          .ReceiveDelay1 = val.value.ReceiveDelay1, .ReceiveDelay2 = val.value.ReceiveDelay2,
          .JoinAcceptDelay1 = val.value.JoinAcceptDelay1, .JoinAcceptDelay2 = val.value.JoinAcceptDelay2 }
{

}

BandDefaults::BandDefaults(
    const BAND_DEFAULTS &val
)
    : value { .RX2Frequency = val.RX2Frequency, .RX2DataRate = val.RX2DataRate,
        .ReceiveDelay1 = val.ReceiveDelay1, .ReceiveDelay2 = val.ReceiveDelay2,
        .JoinAcceptDelay1 = val.JoinAcceptDelay1, .JoinAcceptDelay2 = val.JoinAcceptDelay2 }
{

}

BandDefaults &BandDefaults::operator=(
    const BandDefaults &val
)
{
    value.RX2Frequency = val.value.RX2Frequency;
    value.RX2DataRate = val.value.RX2DataRate;
    value.ReceiveDelay1 = val.value.ReceiveDelay1;
    value.ReceiveDelay2 = val.value.ReceiveDelay2;
    value.JoinAcceptDelay1 = val.value.JoinAcceptDelay1;
    value.JoinAcceptDelay2 = val.value.JoinAcceptDelay2;
    return *this;
}

void BandDefaults::setValue(
    int aRX2Frequency,
    int aRX2DataRate,
    int aReceiveDelay1,
    int aReceiveDelay2,
    int aJoinAcceptDelay1,
    int aJoinAcceptDelay2
) {
    value.RX2Frequency = aRX2Frequency;
    value.RX2DataRate = aRX2DataRate;
    value.ReceiveDelay1 = aReceiveDelay1;
    value.ReceiveDelay2 = aReceiveDelay2;
    value.JoinAcceptDelay1 = aJoinAcceptDelay1;
    value.JoinAcceptDelay2 = aJoinAcceptDelay2;
}

std::string BandDefaults::toString() const
{
    std::stringstream ss;
    ss << "{\"RX2Frequency\": " << value.RX2Frequency
        << ", \"RX2DataRate\": " << value.RX2DataRate
        << ", \"ReceiveDelay1\": " << value.ReceiveDelay1
        << ", \"ReceiveDelay2\": " << value.ReceiveDelay2
        << ", \"JoinAcceptDelay1\": " << value.JoinAcceptDelay1
        << ", \"JoinAcceptDelay2\": " << value.JoinAcceptDelay2
        << "}";
    return ss.str();
}

MaxPayloadSize::MaxPayloadSize()
    : value { .m = 0, .n = 0 }
{

}

MaxPayloadSize::MaxPayloadSize(
    const MaxPayloadSize &val
)
    : value { . m = val.value.m, .n = val.value.n }
{

}

MaxPayloadSize::MaxPayloadSize(
    const MAX_PAYLOAD_SIZE &val
)
    : value { . m = val.m, .n = val.n }
{

}

MaxPayloadSize::MaxPayloadSize(
    uint8_t aM,
    uint8_t aN
)
    : value { .m = aM, .n = aN }
{

}

std::string MaxPayloadSize::toString() const
{
    std::stringstream ss;
    ss << "{\"m\": " << (int) value.m
       << ", \"n\": " << (int) value.n
       << "}";
    return ss.str();
}

void MaxPayloadSize::setValue(uint8_t am, uint8_t an) {
    value.m = am;
    value.n = an;
}

RegionalParameterChannelPlan::RegionalParameterChannelPlan()
    : value { .id = 0, .maxUplinkEIRP = 0.0, .defaultDownlinkTXPower = 0,
        .pingSlotFrequency = 0, .implementsTXParamSetup = false, .defaultRegion = false, .supportsExtraChannels = false
    }
{
}

RegionalParameterChannelPlan::RegionalParameterChannelPlan(
    const REGIONAL_PARAMETER_CHANNEL_PLAN &val
)
    : value {
        .id = val.id, .name = val.name, .cn = val.cn, .maxUplinkEIRP = val.maxUplinkEIRP,
        .defaultDownlinkTXPower = val.defaultDownlinkTXPower,
        .pingSlotFrequency = val.pingSlotFrequency, .implementsTXParamSetup = val.implementsTXParamSetup,
        .defaultRegion = val.defaultRegion, .supportsExtraChannels = val.supportsExtraChannels, .bandDefaults = val.bandDefaults,

        .txPowerOffsets = val.txPowerOffsets,
        .uplinkChannels = val.uplinkChannels,
        .downlinkChannels = val.downlinkChannels
    }
{
    for (int i = 0; i < DATA_RATE_SIZE; i++) {
        value.dataRates[i] = val.dataRates[i];
        value.maxPayloadSizePerDataRate[i] = val.maxPayloadSizePerDataRate[i];
        value.maxPayloadSizePerDataRateRepeater[i] = val.maxPayloadSizePerDataRateRepeater[i];
        value.rx1DataRateOffsets[i] = val.rx1DataRateOffsets[i];
    }
}

RegionalParameterChannelPlan::RegionalParameterChannelPlan(
    const RegionalParameterChannelPlan &val
)
    : value {
        .id = val.value.id, .name = val.value.name, .cn = val.value.cn,
        .maxUplinkEIRP = val.value.maxUplinkEIRP, .defaultDownlinkTXPower = val.value.defaultDownlinkTXPower,
        .pingSlotFrequency = val.value.pingSlotFrequency, .implementsTXParamSetup = val.value.implementsTXParamSetup,
        .defaultRegion = val.value.defaultRegion,
        .supportsExtraChannels = val.value.supportsExtraChannels,
        .bandDefaults = val.value.bandDefaults, .txPowerOffsets = val.value.txPowerOffsets,
        .uplinkChannels = val.value.uplinkChannels, .downlinkChannels = val.value.downlinkChannels }

{
    for (int i = 0; i < DATA_RATE_SIZE; i++ ) {
        value.dataRates[i] = value.dataRates[i];
    }
    for (int i = 0; i < DATA_RATE_SIZE; i++ ) {
        value.maxPayloadSizePerDataRate[i] = value.maxPayloadSizePerDataRate[i];
    }
    for (int i = 0; i < DATA_RATE_SIZE; i++ ) {
        value.maxPayloadSizePerDataRateRepeater[i] = value.maxPayloadSizePerDataRateRepeater[i];
    }
    for (int i = 0; i < DATA_RATE_SIZE; i++ ) {
        value.rx1DataRateOffsets[i] = value.rx1DataRateOffsets[i];
    }
}

template <typename T, typename A>
static void vectorAppendJSON(std::ostream &strm, std::vector<T, A> const &value)
{
    strm << "[";
    bool hasPrev = false;
    for (typename std::vector<T, A>::const_iterator it(value.begin()); it != value.end(); it++) {
        if (hasPrev)
            strm << ", ";
        else
            hasPrev = true;
        strm << it->toString();
    }
    strm << "]";
}

template <typename T>
static void intsAppendJSON(std::ostream &strm, T &value)
{
    strm << "[";
    bool hasPrev = false;
    for (int i = 0; i < DATA_RATE_SIZE; i++) {
        if (hasPrev)
            strm << ", ";
        else
            hasPrev = true;
        strm << (int) value[i];
    }
    strm << "]";
}

static void txPowerOffsetsAppendJSON(std::ostream &strm,
    const std::vector<int8_t> &value)
{
    strm << "[";
    bool hasPrev = false;
    for (int i = 0; i < value.size(); i++) {
        if (hasPrev)
            strm << ", ";
        else
            hasPrev = true;
        strm << (int) value[i];
    }
    strm << "]";
}

template <typename T>
static void arrayAppendJSON(std::ostream &strm, T &value)
{
    strm << "[";
    bool hasPrev = false;
    for (int i = 0; i < DATA_RATE_SIZE; i++) {
        if (hasPrev)
            strm << ", ";
        else
            hasPrev = true;
        strm << value[i].toString();
    }
    strm << "]";
}

static void rx1DataRateOffsetsAppendJSON(std::ostream &strm, const std::vector<uint8_t> (&value)[DATA_RATE_SIZE])
{
    strm << "[";
    bool hasPrev = false;
    for (int i = 0; i < DATA_RATE_SIZE; i++) {
        if (hasPrev)
            strm << ", ";
        else
            hasPrev = true;
        bool hasPrev2 = false;
        strm << "[";
        const std::vector<uint8_t> &v = value[i];
        for (std::vector<uint8_t>::const_iterator it(v.begin()); it != v.end(); it++) {
            if (hasPrev2)
                strm << ", ";
            else
                hasPrev2 = true;
            strm << (int) *it;
        }
        strm << "]";
    }
    strm << "]";
}

std::string RegionalParameterChannelPlan::toString() const
{
    std::stringstream ss;
    ss << "{\"id\": " << (int) value.id
       << ", \"name\": \"" << value.name
       << "\", \"cn\": \"" << value.cn
       << "\", \"implementsTXParamSetup\": " << (value.implementsTXParamSetup ? STR_TRUE_FALSE)
       << ", \"maxUplinkEIRP\": " << value.maxUplinkEIRP
       << ", \"pingSlotFrequency\": " << value.pingSlotFrequency
       << ", \"defaultDownlinkTXPower\": " << value.defaultDownlinkTXPower
       << ", \"supportsExtraChannels\": " << (value.supportsExtraChannels ? STR_TRUE_FALSE)
       << ", \"defaultRegion\": " << (value.defaultRegion ? STR_TRUE_FALSE)
       << ", \"bandDefaults\": " << value.bandDefaults.toString()
       << ", \"dataRates\": ";
    arrayAppendJSON(ss, value.dataRates);
    ss << ", \"uplinkChannels\": ";

    vectorAppendJSON(ss, value.uplinkChannels);
    ss << ", \"downlinkChannels\": ";
    vectorAppendJSON(ss, value.downlinkChannels);

    ss << ", \"maxPayloadSizePerDataRate\": ";
    arrayAppendJSON(ss, value.maxPayloadSizePerDataRate);
    ss << ", \"maxPayloadSizePerDataRateRepeater\": ";
    arrayAppendJSON(ss, value.maxPayloadSizePerDataRateRepeater);
    ss << ", \"rx1DataRateOffsets\": ";
    // ss << "[]";
    rx1DataRateOffsetsAppendJSON(ss, value.rx1DataRateOffsets);
    ss << ", \"txPowerOffsets\": ";
    txPowerOffsetsAppendJSON(ss, value.txPowerOffsets);

    ss << "}";
    return ss.str();
}

void RegionalParameterChannelPlan::setTxPowerOffsets(
    int count, ...
)
{
    if (count >= TX_POWER_OFFSET_MAX_SIZE)
        count = TX_POWER_OFFSET_MAX_SIZE;
    va_list ap;
    va_start(ap, count);
    for (int i = 0; i < count; i++) {
        value.txPowerOffsets.push_back(va_arg(ap, int));
    }
    va_end(ap);
}

void RegionalParameterChannelPlan::setRx1DataRateOffsets(
    int dataRateIndex,
    int count, ...
)
{
    if (dataRateIndex >= DATA_RATE_SIZE)
        return;
    va_list ap;
    va_start(ap, count);
    value.rx1DataRateOffsets[dataRateIndex].clear();
    for (int i = 0; i < count; i++) {
        value.rx1DataRateOffsets[dataRateIndex].push_back(va_arg(ap, int));
    }
    va_end(ap);
}


int RegionalParameterChannelPlan::joinAcceptDelay1() const
{
    return 5; // 5s
}

int RegionalParameterChannelPlan::joinAcceptDelay2() const
{
   return 6; // 6s
}

std::string RegionalParameterChannelPlan::toDescriptionTableString() const {
    std::stringstream ss;
    ss  << std::fixed << std::setprecision(2)
        << "name: " << value.name << std::endl
        << "Frequency, MHz. RX2 " << value.bandDefaults.value.RX2Frequency / 1000000. << std::endl;
    int c = 0;
    for (std::vector<Channel>::const_iterator it(value.uplinkChannels.begin()); it != value.uplinkChannels.end(); it++) {
        ss << "Uplink channel " << c << "    " << it->value.frequency / 1000000. << std::endl;
        c++;
    }
    return ss.str();
}

/*
maxUplinkEIRP
 */
void RegionalParameterChannelPlan::toHeader(
    std::ostream &strm,
    int tabs
) const {
    std::string prefix = std::string(tabs, '\t');
    strm << prefix << "{\n";
    prefix += '\t';
    strm << prefix << ".id = " << (int) value.id << ",\n"
        << prefix << ".name = \"" << value.name << "\",\n"
        << prefix << ".cn = \"" << value.cn << "\",\n"
        << prefix << ".maxUplinkEIRP = " << value.maxUplinkEIRP << ",\n"
        << prefix << ".defaultDownlinkTXPower = " << value.defaultDownlinkTXPower << ",\n"
        << prefix << ".pingSlotFrequency = " << value.pingSlotFrequency << ",\n"
        << prefix << ".implementsTXParamSetup = " << (value.implementsTXParamSetup ? STR_TRUE_FALSE) << ",\n"
        << prefix << ".defaultRegion = " << value.defaultRegion << ",\n"
        << prefix << ".supportsExtraChannels = " << value.supportsExtraChannels << ",\n"
        << prefix << ".bandDefaults = BandDefaults({\n";
    prefix += '\t';
    strm << prefix << ".RX2Frequency = " << value.bandDefaults.value.RX2Frequency << ",\n"
         << prefix << ".RX2DataRate = " << value.bandDefaults.value.RX2DataRate << ",\n"
         << prefix << ".ReceiveDelay1 = " << value.bandDefaults.value.ReceiveDelay1 << ",\n"
         << prefix << ".JoinAcceptDelay1 = " << value.bandDefaults.value.JoinAcceptDelay1 << ",\n"
         << prefix << ".JoinAcceptDelay2 = " << value.bandDefaults.value.JoinAcceptDelay2 << "\n";
    prefix.erase(prefix.size() - 1);
    strm << prefix << "}),\n";

    strm << prefix << ".dataRates = {";
    prefix += '\t';
    bool f = true;
    for (int i = 0; i < DATA_RATE_SIZE; i++) {
        if (f) {
            f = false;
            strm << "\n";
        } else
            strm << ",\n";
        strm << prefix << "DataRate({\n";
        prefix += '\t';
        strm << prefix << ".uplink = " << (value.dataRates[i].value.uplink ? STR_TRUE_FALSE) << ",\n"
             << prefix << ".downlink = " << (value.dataRates[i].value.downlink ? STR_TRUE_FALSE) << ",\n"
             << prefix << ".modulation = (MODULATION) " << (int) value.dataRates[i].value.modulation << ",\n"
             << prefix << ".bandwidth = (BANDWIDTH) " << (int) value.dataRates[i].value.bandwidth << ",\n"
             << prefix << ".spreadingFactor = (SPREADING_FACTOR) " << (int) value.dataRates[i].value.spreadingFactor << "\n";
        prefix.erase(prefix.size() - 1);
        strm << prefix << "})";
    };
    prefix.erase(prefix.size() - 1);
    strm << "\n" << prefix << "},\n";

    strm << prefix << ".maxPayloadSizePerDataRate = {";
    prefix += '\t';
    f = true;
    for (int i = 0; i < DATA_RATE_SIZE; i++) {
        if (f) {
            f = false;
            strm << "\n";
        } else
            strm << ",\n";
        strm << prefix << "MaxPayloadSize({\n";
        prefix += '\t';
        strm << prefix << ".m = " << (int) value.maxPayloadSizePerDataRate[i].value.m << ",\n"
             << prefix << ".n = " << (int) value.maxPayloadSizePerDataRate[i].value.n << "\n";
        prefix.erase(prefix.size() - 1);
        strm << prefix << "})";
    };
    prefix.erase(prefix.size() - 1);
    strm << '\n' << prefix << "},\n";

    strm << prefix << ".maxPayloadSizePerDataRateRepeater = {";
    prefix += '\t';
    f = true;
    for (int i = 0; i < DATA_RATE_SIZE; i++) {
        if (f) {
            f = false;
            strm << "\n";
        } else
            strm << ",\n";
        strm << prefix << "{\n";
        prefix += '\t';
        strm << prefix << ".m = " << (int) value.maxPayloadSizePerDataRateRepeater[i].value.m << ",\n"
             << prefix << ".n = " << (int) value.maxPayloadSizePerDataRateRepeater[i].value.n << "\n";
        prefix.erase(prefix.size() - 1);
        strm << prefix << "}";
    };
    prefix.erase(prefix.size() - 1);
    strm << '\n' << prefix << "},\n";

    strm << prefix << ".rx1DataRateOffsets = {";
    prefix += '\t';
    f = true;
    for (int i = 0; i < DATA_RATE_SIZE; i++) {
        if (f) {
            f = false;
        } else
            strm << ", ";

        strm << "{";

        bool f1 = true;
        for (auto & rxDROffset: value.rx1DataRateOffsets[i]) {
            if (f1) {
                f1 = false;
            } else
                strm << ", ";
            strm << (int) rxDROffset;
        }
        strm << "}";
    };
    strm << "},\n";
    prefix.erase(prefix.size() - 1);

    strm << prefix << ".txPowerOffsets = {";
    prefix += '\t';
    f = true;
    for (auto &txPO: value.txPowerOffsets) {
        if (f) {
            f = false;
        } else
            strm << ", ";
        strm << (int) txPO;
    };
    prefix.erase(prefix.size() - 1);
    strm << "}\n";
    prefix.erase(prefix.size() - 1);

    strm << prefix << ".uplinkChannels = {";
    prefix += '\t';
    f = true;
    for (auto &uplink: value.uplinkChannels) {
        if (f) {
            f = false;
            strm << "\n";
        } else
            strm << ",\n";
        strm << prefix << "{\n";
        prefix += '\t';
        strm << prefix << ".frequency = " << uplink.value.frequency << ",\n"
            << prefix << ".minDR = " << uplink.value.minDR << ",\n"
            << prefix << ".maxDR = " << uplink.value.maxDR << ",\n"
            << prefix << ".enabled = " << (uplink.value.enabled ? STR_TRUE_FALSE) << ",\n"
            << prefix << ".custom = " << (uplink.value.custom ? STR_TRUE_FALSE) << "\n";
        prefix.erase(prefix.size() - 1);
        strm << prefix << "}";
    };
    prefix.erase(prefix.size() - 1);
    strm << '\n' << prefix << "},\n";

    strm << prefix << ".downlinkChannels = {";
    prefix += '\t';
    f = true;
    for (auto &downlink: value.downlinkChannels) {
        if (f) {
            f = false;
            strm << "\n";
        } else
            strm << ",\n";
        strm << prefix << "{\n";
        prefix += '\t';
        strm << prefix << ".frequency = " << downlink.value.frequency << ",\n"
             << prefix << ".minDR = " << downlink.value.minDR << ",\n"
             << prefix << ".maxDR = " << downlink.value.maxDR << ",\n"
             << prefix << ".enabled = " << (downlink.value.enabled ? STR_TRUE_FALSE) << ",\n"
             << prefix << ".custom = " << (downlink.value.custom ? STR_TRUE_FALSE) << "\n";
        prefix.erase(prefix.size() - 1);
        strm << prefix << "}";
    };
    prefix.erase(prefix.size() - 1);
    strm << '\n' << prefix << "},\n";

    strm << prefix << "}\n";
}

const RegionalParameterChannelPlan* RegionBands::get(const std::string &name) const
{
    for (std::vector<RegionalParameterChannelPlan>::const_iterator it(bands.begin()); it != bands.end(); it++) {
        if (it->value.name == name) {
            return &*it;
        }
    }
    return nullptr;
}

std::string RegionBands::toString() const {
    std::stringstream ss;
    ss << "{\"regionalParametersVersion\": \"" << REGIONAL_PARAMETERS_VERSION2string(regionalParametersVersion)
       << "\", \"RegionBands\": ";
    vectorAppendJSON(ss, bands);
    ss << "}";
    return ss.str();
}

bool RegionBands::setRegionalParametersVersion(const std::string &value)
{
    regionalParametersVersion = string2REGIONAL_PARAMETERS_VERSION(value);
    return true;
}

static bool isAnyLorawanVersion(LORAWAN_VERSION value) {
    return *(uint8_t*) &value == 0;
}

static bool isAnyRegionalParametersVersion(REGIONAL_PARAMETERS_VERSION value) {
    return *(uint8_t*) &value == 0;
}

RegionBands::RegionBands()
    : regionalParametersVersion({ 0, 0, 0 })
{
}

RegionBands::RegionBands(const RegionBands &value)
    : regionalParametersVersion(value.regionalParametersVersion), bands(value.bands)
{
}

RegionBands::RegionBands(
    const std::vector<REGIONAL_PARAMETER_CHANNEL_PLAN> &aBands
)
    : regionalParametersVersion({ 0, 0, 0 })
{
    for(auto b : aBands) {
        bands.push_back(RegionalParameterChannelPlan(b));
    }
}
