#include <sstream>
#include <cstdarg>
#include <iomanip>
#include <algorithm>

#include "lorawan/regional-parameters/regional-parameter-channel-plan.h"
#include "lorawan/lorawan-string.h"

Channel::Channel()
    : value { 0, 0, 0, true, false }
{
}

Channel::Channel(const Channel &val)
    : value { val.value.frequency, val.value.minDR, val.value.maxDR, val.value.enabled, val.value.custom }
{
}

Channel::Channel(
    const RADIO_CHANNEL &val
)
    : value { val.frequency, val.minDR, val.maxDR, val.enabled, val.custom }
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
    : value { 0, 0, 0, 0, 0, 0 }
{
}

BandDefaults::BandDefaults(const BandDefaults& val)
    : value { val.value.RX2Frequency, val.value.RX2DataRate, val.value.ReceiveDelay1,
    val.value.ReceiveDelay2, val.value.JoinAcceptDelay1, val.value.JoinAcceptDelay2 }
{
}

BandDefaults::BandDefaults(
    const BAND_DEFAULTS& val
)
    : value { val.RX2Frequency, val.RX2DataRate, val.ReceiveDelay1,
    val.ReceiveDelay2, val.JoinAcceptDelay1, val.JoinAcceptDelay2 }
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
    : value { 0, 0 }
{
}

MaxPayloadSize::MaxPayloadSize(
    const MAX_PAYLOAD_SIZE& val
)
    : value { val.m, val.n }
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

REGIONAL_PARAMETER_CHANNEL_PLAN &REGIONAL_PARAMETER_CHANNEL_PLAN::operator=(
    const REGIONAL_PARAMETER_CHANNEL_PLAN &value
) {
    id = value.id;                                          // 1..14
    subRegion = value.subRegion;
    name = value.name;                                      // channel plan name
    cn = value.cn;                                          // common name
    maxUplinkEIRP = value.maxUplinkEIRP;                    // dBm default
    defaultDownlinkTXPower = value.defaultDownlinkTXPower;  // can depend on frequency
    pingSlotFrequency = value.pingSlotFrequency;
    implementsTXParamSetup = value.implementsTXParamSetup;
    defaultRegion = value.defaultRegion;                    // true- default region
    supportsExtraChannels = value.supportsExtraChannels;
    bandDefaults = value.bandDefaults;                      //
    dataRates = value.dataRates;
    maxPayloadSizePerDataRate = value.maxPayloadSizePerDataRate;
    maxPayloadSizePerDataRateRepeater = value.maxPayloadSizePerDataRateRepeater;
    rx1DataRateOffsets = value.rx1DataRateOffsets;
    // Max EIRP - <offset> dB, 0..16
    txPowerOffsets = value.txPowerOffsets;
    uplinkChannels = value.uplinkChannels;
    downlinkChannels = value.downlinkChannels;
    return *this;
}

RegionalParameterChannelPlan::RegionalParameterChannelPlan()
    : value { 0, 0, "", "", 0.0, 0, false, false, false }
{
}

RegionalParameterChannelPlan::RegionalParameterChannelPlan(
    const REGIONAL_PARAMETER_CHANNEL_PLAN &val
)
    : value { val.id, val.subRegion, val.name, val.cn, val.maxUplinkEIRP,
        val.defaultDownlinkTXPower, val.pingSlotFrequency, val.implementsTXParamSetup,
        val.defaultRegion, val.supportsExtraChannels, val.bandDefaults,
        val.dataRates, val.maxPayloadSizePerDataRate, val.maxPayloadSizePerDataRateRepeater,
        val.rx1DataRateOffsets, val.txPowerOffsets, val.uplinkChannels, val.downlinkChannels }
{
}

RegionalParameterChannelPlan::RegionalParameterChannelPlan(
    const RegionalParameterChannelPlan& val
)
: value { val.value.id, val.value.subRegion, val.value.name, val.value.cn,
    val.value.maxUplinkEIRP,val.value.defaultDownlinkTXPower,
    val.value.pingSlotFrequency,
    val.value.implementsTXParamSetup,
    val.value.defaultRegion, val.value.supportsExtraChannels,
    val.value.bandDefaults,
    val.value.dataRates, val.value.maxPayloadSizePerDataRate,
    val.value.maxPayloadSizePerDataRateRepeater,
    val.value.rx1DataRateOffsets, val.value.txPowerOffsets,
    val.value.uplinkChannels, val.value.downlinkChannels }
{
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
    for (int i = 0; i < value.size(); i++) {
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
    for (auto powerOfs : value) {
        if (hasPrev)
            strm << ", ";
        else
            hasPrev = true;
        strm << (int) powerOfs;
    }
    strm << "]";
}

template <typename T>
static void arrayAppendJSON(std::ostream &strm, T &value)
{
    strm << "[";
    bool hasPrev = false;
    for (int i = 0; i < value.size(); i++) {
        if (hasPrev)
            strm << ", ";
        else
            hasPrev = true;
        strm << value[i].toString();
    }
    strm << "]";
}

static void rx1DataRateOffsetsAppendJSON(
    std::ostream &strm,
    const std::vector <std::vector<uint8_t> > &value)
{
    strm << "[";
    bool hasPrev = false;
    for (const auto & rateOffsets : value) {
        if (hasPrev)
            strm << ", ";
        else
            hasPrev = true;
        bool hasPrev2 = false;
        strm << "[";
        for (auto rateOffset : rateOffsets) {
            if (hasPrev2)
                strm << ", ";
            else
                hasPrev2 = true;
            strm << (int) rateOffset;
        }
        strm << "]";
    }
    strm << "]";
}

std::string RegionalParameterChannelPlan::toString() const
{
    std::stringstream ss;
    ss << "{\"id\": " << (int) value.id
        << R"(, "subRegion": ")" << value.subRegion
        << R"(, "name": ")" << value.name
        << R"(", "cn": ")" << value.cn
        << R"(", "implementsTXParamSetup": )" << (value.implementsTXParamSetup ? STR_TRUE_FALSE)
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
    if (dataRateIndex >= value.rx1DataRateOffsets.size())
        return;
    va_list ap;
    va_start(ap, count);
    value.rx1DataRateOffsets[dataRateIndex].clear();
    for (int i = 0; i < count; i++) {
        value.rx1DataRateOffsets[dataRateIndex].push_back(va_arg(ap, int));
    }
    va_end(ap);
}

const REGIONAL_PARAMETER_CHANNEL_PLAN* RegionalParameterChannelPlan::get() const
{
    return &value;
}

REGIONAL_PARAMETER_CHANNEL_PLAN* RegionalParameterChannelPlan::mut()
{
    return &value;
}

void RegionalParameterChannelPlan::set(
    const REGIONAL_PARAMETER_CHANNEL_PLAN &val
) {
    value = val;
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
    for (const auto & uplinkChannel : value.uplinkChannels) {
        ss << "Uplink channel " << c << "    " << uplinkChannel.value.frequency / 1000000. << std::endl;
        c++;
    }
    return ss.str();
}

/*
maxUplinkEIRP
 */
void RegionalParameterChannelPlan::toHeader(
    std::ostream &strm,
    int tabs,
    bool cpp20
) const {
    std::string prefix = std::string(tabs, '\t');
    strm << prefix << "{\n";
    prefix += '\t';
    if (cpp20) {
        strm << prefix << ".id = " << (int) value.id << ",\n"
            << prefix << ".subRegion = " << (int) value.subRegion << ",\n"
            << prefix << ".name = \"" << value.name << "\",\n"
            << prefix << ".cn = \"" << value.cn << "\",\n"
            << prefix << std::fixed << std::setprecision(2) << ".maxUplinkEIRP = " << value.maxUplinkEIRP << "f,\n"
            << prefix << ".defaultDownlinkTXPower = " << value.defaultDownlinkTXPower << ",\n"
            << prefix << ".pingSlotFrequency = " << value.pingSlotFrequency << ",\n"
            << prefix << ".implementsTXParamSetup = " << (value.implementsTXParamSetup ? STR_TRUE_FALSE) << ",\n"
            << prefix << ".defaultRegion = " << (value.defaultRegion ? STR_TRUE_FALSE) << ",\n"
            << prefix << ".supportsExtraChannels = " << (value.supportsExtraChannels ? STR_TRUE_FALSE) << ",\n"
            << prefix << ".bandDefaults = BandDefaults({\n";
        prefix += '\t';
        strm << prefix << ".RX2Frequency = " << value.bandDefaults.value.RX2Frequency << ",\n"
            << prefix << ".RX2DataRate = " << value.bandDefaults.value.RX2DataRate << ",\n"
            << prefix << ".ReceiveDelay1 = " << value.bandDefaults.value.ReceiveDelay1 << ",\n"
            << prefix << ".ReceiveDelay2 = " << value.bandDefaults.value.ReceiveDelay2 << ",\n"
            << prefix << ".JoinAcceptDelay1 = " << value.bandDefaults.value.JoinAcceptDelay1 << ",\n"
            << prefix << ".JoinAcceptDelay2 = " << value.bandDefaults.value.JoinAcceptDelay2 << "\n";
        prefix.erase(prefix.size() - 1);
        strm << prefix << "}),\n";

        strm << prefix << ".dataRates = {";
        prefix += '\t';
        bool f = true;
        for (const auto & dataRate : value.dataRates) {
            if (f) {
                f = false;
                strm << "\n";
            } else
                strm << ",\n";
            strm << prefix << "DataRate({\n";
            prefix += '\t';
            strm << prefix << ".uplink = " << (dataRate.value.uplink ? STR_TRUE_FALSE) << ",\n"
                 << prefix << ".downlink = " << (dataRate.value.downlink ? STR_TRUE_FALSE) << ",\n"
                 << prefix << ".modulation = (MODULATION) " << (int) dataRate.value.modulation << ",\n"
                 << prefix << ".bandwidth = (BANDWIDTH) " << (int) dataRate.value.bandwidth << ",\n"
                 << prefix << ".spreadingFactor = (SPREADING_FACTOR) " << (int) dataRate.value.spreadingFactor
                 << ",\n"
                 << prefix << ".bps = " << dataRate.value.bps << "\n";
            prefix.erase(prefix.size() - 1);
            strm << prefix << "})";
        };
        prefix.erase(prefix.size() - 1);
        strm << "\n" << prefix << "},\n";

        strm << prefix << ".maxPayloadSizePerDataRate = {";
        prefix += '\t';
        f = true;
        for (const auto & maxPayloadSize : value.maxPayloadSizePerDataRate) {
            if (f) {
                f = false;
                strm << "\n";
            } else
                strm << ",\n";
            strm << prefix << "MaxPayloadSize({\n";
            prefix += '\t';
            strm << prefix << ".m = " << (int) maxPayloadSize.value.m << ",\n"
                 << prefix << ".n = " << (int) maxPayloadSize.value.n << "\n";
            prefix.erase(prefix.size() - 1);
            strm << prefix << "})";
        };
        prefix.erase(prefix.size() - 1);
        strm << '\n' << prefix << "},\n";

        strm << prefix << ".maxPayloadSizePerDataRateRepeater = {";
        prefix += '\t';
        f = true;
        for (const auto & maxPayloadSizeRpt : value.maxPayloadSizePerDataRateRepeater) {
            if (f) {
                f = false;
                strm << "\n";
            } else
                strm << ",\n";
            strm << prefix << "MaxPayloadSize({\n";
            prefix += '\t';
            strm << prefix << ".m = " << (int) maxPayloadSizeRpt.value.m << ",\n"
                 << prefix << ".n = " << (int) maxPayloadSizeRpt.value.n << "\n";
            prefix.erase(prefix.size() - 1);
            strm << prefix << "})";
        };
        prefix.erase(prefix.size() - 1);
        strm << '\n' << prefix << "},\n";

        strm << prefix << ".rx1DataRateOffsets = {";
        prefix += '\t';
        f = true;
        for (const auto &rx1DataRateOffset: value.rx1DataRateOffsets) {
            if (f) {
                f = false;
            } else
                strm << ", ";

            strm << "{";

            bool f1 = true;
            for (auto &rxDROffset: rx1DataRateOffset) {
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
        strm << "},\n";

        strm << prefix << ".uplinkChannels = {";
        prefix += '\t';
        f = true;
        for (auto &uplink: value.uplinkChannels) {
            if (f) {
                f = false;
                strm << "\n";
            } else
                strm << ",\n";
            strm << prefix << "Channel({\n";
            prefix += '\t';
            strm << prefix << ".frequency = " << uplink.value.frequency << ",\n"
                 << prefix << ".minDR = " << uplink.value.minDR << ",\n"
                 << prefix << ".maxDR = " << uplink.value.maxDR << ",\n"
                 << prefix << ".enabled = " << (uplink.value.enabled ? STR_TRUE_FALSE) << ",\n"
                 << prefix << ".custom = " << (uplink.value.custom ? STR_TRUE_FALSE) << "\n";
            prefix.erase(prefix.size() - 1);
            strm << prefix << "})";
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
            strm << prefix << "Channel({\n";
            prefix += '\t';
            strm << prefix << ".frequency = " << downlink.value.frequency << ",\n"
                 << prefix << ".minDR = " << downlink.value.minDR << ",\n"
                 << prefix << ".maxDR = " << downlink.value.maxDR << ",\n"
                 << prefix << ".enabled = " << (downlink.value.enabled ? STR_TRUE_FALSE) << ",\n"
                 << prefix << ".custom = " << (downlink.value.custom ? STR_TRUE_FALSE) << "\n";
            prefix.erase(prefix.size() - 1);
            strm << prefix << "})";
        };
        prefix.erase(prefix.size() - 1);
        strm << '\n' << prefix << "}\n";
        prefix.erase(prefix.size() - 1);
        strm << prefix << "}";
    } else {
        // C++11
        strm << prefix << (int) value.id << ", // id\n"
            << prefix << (int) value.subRegion << ", // subRegion\n"
            << prefix << "\"" << value.name << "\", // name\n"
            << prefix << "\"" << value.cn << "\", // cn \n"
            << prefix << std::fixed << std::setprecision(2) << value.maxUplinkEIRP << "f, // maxUplinkEIRP\n"
            << prefix << value.defaultDownlinkTXPower << ", // defaultDownlinkTXPower\n"
            << prefix << value.pingSlotFrequency << ", // pingSlotFrequency\n"
            << prefix << (value.implementsTXParamSetup ? STR_TRUE_FALSE) << ", // implementsTXParamSetup\n"
            << prefix << (value.defaultRegion ? STR_TRUE_FALSE) << ", // defaultRegion\n"
            << prefix << (value.supportsExtraChannels ? STR_TRUE_FALSE) << ", // supportsExtraChannels\n"
            << prefix << "BandDefaults({ // bandDefaults\n";
        prefix += '\t';
        strm << prefix << value.bandDefaults.value.RX2Frequency << ", // RX2Frequency\n"
            << prefix << value.bandDefaults.value.RX2DataRate << ", // RX2DataRate\n"
            << prefix << value.bandDefaults.value.ReceiveDelay1 << ", // ReceiveDelay1\n"
            << prefix << value.bandDefaults.value.ReceiveDelay2 << ", // ReceiveDelay2\n"
            << prefix << value.bandDefaults.value.JoinAcceptDelay1 << ", // JoinAcceptDelay1\n"
            << prefix << value.bandDefaults.value.JoinAcceptDelay2 << " // JoinAcceptDelay2\n";
        prefix.erase(prefix.size() - 1);
        strm << prefix << "}),\n";

        strm << prefix << "{ // dataRates";
        prefix += '\t';
        bool f = true;
        for (const auto & dataRate : value.dataRates) {
            if (f) {
                f = false;
                strm << "\n";
            } else
                strm << ",\n";
            strm << prefix << "DataRate({\n";
            prefix += '\t';
            strm << prefix << (dataRate.value.uplink ? STR_TRUE_FALSE) << ", // uplink\n"
                 << prefix << (dataRate.value.downlink ? STR_TRUE_FALSE) << ", // downlink\n"
                 << prefix << "(MODULATION) " << (int) dataRate.value.modulation << ", // modulation\n"
                 << prefix << "(BANDWIDTH) " << (int) dataRate.value.bandwidth << ", // bandwidth\n"
                 << prefix << "(SPREADING_FACTOR) " << (int) dataRate.value.spreadingFactor
                 << ", // spreadingFactor\n"
                 << prefix << dataRate.value.bps << " // bps\n";
            prefix.erase(prefix.size() - 1);
            strm << prefix << "})";
        };
        prefix.erase(prefix.size() - 1);
        strm << "\n" << prefix << "},\n";

        strm << prefix << "{ // maxPayloadSizePerDataRate";
        prefix += '\t';
        f = true;
        for (const auto & maxPayloadSizePerDataRate : value.maxPayloadSizePerDataRate) {
            if (f) {
                f = false;
                strm << "\n";
            } else
                strm << ",\n";
            strm << prefix << "MaxPayloadSize({\n";
            prefix += '\t';
            strm << prefix << (int) maxPayloadSizePerDataRate.value.m << ", // m\n"
                 << prefix << (int) maxPayloadSizePerDataRate.value.n << " // n\n";
            prefix.erase(prefix.size() - 1);
            strm << prefix << "})";
        };
        prefix.erase(prefix.size() - 1);
        strm << '\n' << prefix << "},\n";

        strm << prefix << "{ // maxPayloadSizePerDataRateRepeater";
        prefix += '\t';
        f = true;
        for (const auto & maxPayloadSizePerDataRateRpt : value.maxPayloadSizePerDataRateRepeater) {
            if (f) {
                f = false;
                strm << "\n";
            } else
                strm << ",\n";
            strm << prefix << "MaxPayloadSize({\n";
            prefix += '\t';
            strm << prefix << (int) maxPayloadSizePerDataRateRpt.value.m << ", // m\n"
                 << prefix << (int) maxPayloadSizePerDataRateRpt.value.n << " // n\n";
            prefix.erase(prefix.size() - 1);
            strm << prefix << "})";
        };
        prefix.erase(prefix.size() - 1);
        strm << '\n' << prefix << "},\n";

        strm << prefix << "// rx1DataRateOffsets\n"
            << prefix << "{";
        prefix += '\t';
        f = true;
        for (const auto &rx1DataRateOffset: value.rx1DataRateOffsets) {
            if (f) {
                f = false;
            } else
                strm << ", ";

            strm << "{";

            bool f1 = true;
            for (auto &rxDROffset: rx1DataRateOffset) {
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

        strm << prefix << "// txPowerOffsets\n";
        strm << prefix << "{";
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
        strm << "},\n";

        strm << prefix << "// uplinkChannels\n";
        strm << prefix << "{";
        prefix += '\t';
        f = true;
        for (auto &uplink: value.uplinkChannels) {
            if (f) {
                f = false;
                strm << "\n";
            } else
                strm << ",\n";
            strm << prefix << "Channel({\n";
            prefix += '\t';
            strm << prefix << uplink.value.frequency << ", // frequency\n"
                 << prefix << uplink.value.minDR << ", // minDR\n"
                 << prefix << uplink.value.maxDR << ", // maxDR\n"
                 << prefix << (uplink.value.enabled ? STR_TRUE_FALSE) << ", // enabled\n"
                 << prefix << (uplink.value.custom ? STR_TRUE_FALSE) << " // custom\n";
            prefix.erase(prefix.size() - 1);
            strm << prefix << "})";
        };
        prefix.erase(prefix.size() - 1);
        strm << '\n' << prefix << "},\n";

        strm << prefix << "{ // downlinkChannels";
        prefix += '\t';
        f = true;
        for (auto &downlink: value.downlinkChannels) {
            if (f) {
                f = false;
                strm << "\n";
            } else
                strm << ",\n";
            strm << prefix << "Channel({\n";
            prefix += '\t';
            strm << prefix << downlink.value.frequency << ", // frequency\n"
                 << prefix << downlink.value.minDR << ", // minDR\n"
                 << prefix << downlink.value.maxDR << ", // maxDR\n"
                 << prefix << (downlink.value.enabled ? STR_TRUE_FALSE) << ", // enabled\n"
                 << prefix << (downlink.value.custom ? STR_TRUE_FALSE) << " // custom\n";
            prefix.erase(prefix.size() - 1);
            strm << prefix << "})";
        };
        prefix.erase(prefix.size() - 1);
        strm << '\n' << prefix << "}\n";
        prefix.erase(prefix.size() - 1);
        strm << prefix << "}";
    }
}

void RegionalParameterChannelPlan::get(
    size_t messageSize,
    uint32_t &freqHz,
    int &pwr,
    BANDWIDTH &bandwidth,
    SPREADING_FACTOR &spreadingFactor,
    CODING_RATE &codingRate,
    uint8_t &fdev,
    bool &invert_pol,
    uint16_t &preamble_size,
    bool &no_crc
) const
{
    freqHz = value.pingSlotFrequency;
    for (auto &d: value.downlinkChannels) {
        if (d.value.enabled) {
            freqHz = d.value.frequency;
            break;
        }
    }

    pwr = value.defaultDownlinkTXPower;
    int idx = 7;
    for (int i = 0; i < 8; i++) {
        if (messageSize <= value.maxPayloadSizePerDataRate[i].value.m) {
            idx = i;
            break;
        }
    }
    bandwidth = value.dataRates[idx].value.bandwidth;
    spreadingFactor = value.dataRates[idx].value.spreadingFactor;
    codingRate = CRLORA_4_6;
    fdev = 0;
    invert_pol = false;
    preamble_size = 0;
    no_crc = false;
}

const RegionalParameterChannelPlan* RegionBands::get(const std::string &name) const
{
    std::string upperName(name);
    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
    for (const auto & band : bands) {
        auto *rs = band.get();
        std::string valUpperCN(rs->cn);
        std::transform(valUpperCN.begin(), valUpperCN.end(), valUpperCN.begin(), ::toupper);
        std::string valUpperName(rs->name);
        std::transform(valUpperName.begin(), valUpperName.end(), valUpperName.begin(), ::toupper);
        if (valUpperCN.find(upperName) != std::string::npos)
            return &band;
        if (valUpperName.find(upperName) != std::string::npos)
            return &band;
    }
    return nullptr;
}

std::string RegionBands::toString() const {
    std::stringstream ss;
    ss << R"({"regionalParametersVersion": ")" << REGIONAL_PARAMETERS_VERSION2string(regionalParametersVersion)
       << R"(", "RegionBands": )";
    vectorAppendJSON(ss, bands);
    ss << "}";
    return ss.str();
}

bool RegionBands::setRegionalParametersVersion(const std::string &value)
{
    regionalParametersVersion = string2REGIONAL_PARAMETERS_VERSION(value);
    return true;
}

static bool isAnyLorawanVersion(
    LORAWAN_VERSION value
) {
    return *(uint8_t*) &value == 0;
}

static bool isAnyRegionalParametersVersion(
    REGIONAL_PARAMETERS_VERSION value
) {
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
    for(const auto& b : aBands) {
        bands.push_back(RegionalParameterChannelPlan(b));
    }
}
