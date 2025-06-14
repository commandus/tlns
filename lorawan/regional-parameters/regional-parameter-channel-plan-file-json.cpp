#include <fstream>
#include <iostream>
#include <sstream>

#include "nlohmann/json.hpp"

#include "lorawan/regional-parameters/regional-parameter-channel-plan-file-json.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-msg.h"

/**
 * 	JSON attribute names
 */
#define ATTRS_GATEWAY_CONFIG_COUNT	39

enum JSON_STATE_GATEWAY_CONFIG {
    JRB_NONE = 0,                  	        // Initial state
    JRB_ROOT = 1,                           //
    JRB_BANDS = 2,                 	        // Array of bands

    JRB_BAND = 3,        	                // Band
    JRB_BAND_DEFAULTS = 4,         	        //     Band defaults
    JRB_BAND_DATA_RATES = 5,                // Array
    JRB_BAND_DATA_RATE = 6,           	    //     Data rate
    JRB_BAND_P_SIZES = 7,               	//     Array
    JRB_BAND_P_SIZE = 8,	                //          Max payload size
    JRB_BAND_PRPT_SIZES = 9,                //     Array
    JRB_BAND_PRPT_SIZE = 10,	            //          Max payload size
    JRB_BAND_RX1_DR_OFFSETS = 11,        	//     Array
    JRB_BAND_RX1_DR_OFFSET = 12,            //              Array
    JRB_BAND_TX_POWER_OFS = 13,        	    //     Array
    JRB_BAND_UPLINKS = 14,           	    //     Array
    JRB_BAND_DOWNLINKS = 15,           	    //     Array
    JRB_BAND_CHANNEL = 16              	    //          Channel
};

#define JSON_STATE_GATEWAY_CONFIG_COUNT 17

static const char *PARSE_STATE_NAMES[JSON_STATE_GATEWAY_CONFIG_COUNT] = {
    "none",
    "root",
    "RegionBands",
    "RegionBands[]",
    "bandDefaults",
    "band_data_rates",
    "band_data_rate",
    "band_p_sizes",
    "band_p_size",
    "maxPayloadSizePerDataRate",
    "maxPayloadSizePerDataRateRepeater",
    "rx1DataRateOffsets",
    "rx1DataRateOffsets[]",
    "txPowerOffsets",
    "uplinkChannels",
    "downlinkChannels",
    "up/downlinkChannels[]"
};

enum JSON_KEY_GATEWAY_CONFIG {
    JK_NONE = 0,
    JK_REGIONALPARAMETERSVERSION = 1,
    JK_REGIONBANDS = 2,
    JK_ID = 3,
    JK_NAME = 4,

    JK_CN = 5,
    JK_MAXUPLINKEIRP = 6,
    JK_DEFAULTDOWNLINKTXPOWER = 7,
    JK_PINGSLOTFREQUENCY = 8,
    JK_IMPLEMENTSTXPARAMSETUP = 9,

    JK_SUPPORTSEXTRACHANNELS = 10,
    JK_DEFAULT_REGION = 11,
    JK_BANDDEFAULTS = 12,
    JK_DATA_RATES = 13,
    JK_UPLINKCHANNELS = 14,
    JK_DOWNLINKCHANNELS = 15,
    JK_MAXPAYLOADSIZEPERDATARATE = 16,
    JK_MAXPAYLOADSIZEPERDATARATEREPEATOR = 17,
    JK_RX1DATARATEOFFSETS = 18,
    JK_TXPOWEROFFSETS = 19,

    JK_RX2FREQUENCY = 20,
    JK_RX2DATARATE = 21,
    JK_RECEIVEDELAY1 = 22,
    JK_RECEIVEDELAY2 = 23,
    JK_JOINACCEPTDELAY1 = 24,
    JK_JOINACCEPTDELAY2 = 25,

    JK_UPLINK = 26,
    JK_DOWNLINK = 27,
    JK_MODULATION = 28,
    JK_BANDWIDTH = 29,
    JK_SPREADINGFACTOR = 30,
    JK_BPS = 31,

    JK_M = 32,
    JK_N = 33,

    JK_FREQUENCY = 34,
    JK_MINDR = 35,
    JK_MAXDR = 36,
    JK_ENABLED = 37,
    JK_CUSTOM = 38
};

static const char *ATTR_REGION_BAND_NAMES[ATTRS_GATEWAY_CONFIG_COUNT] = {
        "",
        "regionalParametersVersion",         // 1
        "RegionBands",                       // 2
        "id",                                // 3
        "name",                              // 4

        "cn",                                // 5
        "maxUplinkEIRP",                     // 6
        "defaultDownlinkTXPower",            // 7
        "pingSlotFrequency",                 // 8
        "implementsTXParamSetup",            // 9

        "supportsExtraChannels",             // 10
        "defaultRegion",                     // 11
        "bandDefaults",                      // 12
        "dataRates",                         // 13
        "uplinkChannels",                    // 14
        "downlinkChannels",                  // 15
        "maxPayloadSizePerDataRate",         // 16
        "maxPayloadSizePerDataRateRepeater", // 17
        "rx1DataRateOffsets",                // 18
        "txPowerOffsets",                    // 19
        "RX2Frequency",                      // 20
        "RX2DataRate",                       // 21
        "ReceiveDelay1",                     // 22
        "ReceiveDelay2",                     // 23
        "JoinAcceptDelay1",                  // 24
        "JoinAcceptDelay2",                  // 25

        "uplink",                            // 26
        "downlink",                          // 27
        "modulation",                        // 28
        "bandwidth",                         // 29
        "spreadingFactor",                   // 30
        "bps",                               // 31

        "m",                                 // 32
        "n",                                 // 33

        "frequency",                         // 34
        "minDR",                             // 35
        "maxDR",                             // 36
        "enabled",                           // 37
        "custom"                             // 38
};

static JSON_KEY_GATEWAY_CONFIG getAttrByName(
        const char *name
)
{
    for (int i = 1; i < ATTRS_GATEWAY_CONFIG_COUNT; i++) {
        if (strcmp(ATTR_REGION_BAND_NAMES[i], name) == 0)
            return (JSON_KEY_GATEWAY_CONFIG) i;
    }
    return JK_NONE;
}

RegionalParameterChannelPlanFileJson::RegionalParameterChannelPlanFileJson()
	: errCode(0), defaultRegionBand(nullptr)
{

}

RegionalParameterChannelPlanFileJson::~RegionalParameterChannelPlanFileJson()
{
	done();
}

/**
 *
 * Loads NetworkIdentities
 *	[
 *		{
 *	 		"addr": "network address (hex string, 4 bytes)"
 * 			"eui": "device identifier (hex string, 8 bytes)",
 * 			"nwkSKey": "shared session key (hex string, 16 bytes)",
 *			"appSKey": "private key (hex string, 16 bytes)"
 *		},
 *		..
 *	]
 */
class RegionBandsJsonHandler : public nlohmann::json::json_sax_t {
private:
    RegionalParameterChannelPlanFileJson *value;
    JSON_KEY_GATEWAY_CONFIG keyIndex;
    JSON_STATE_GATEWAY_CONFIG state;
    JSON_STATE_GATEWAY_CONFIG prevState;

    int bandCount, dataRateCount, maxPayloadSizePerDataRateCount,
        maxPayloadSizePerDataRateRepeaterCount,
        rx1DataRateOffsetCount, txPowerOffsetCount,
        uplinkChannelCount, downlinkChannelCount;

    bool Integer(int64_t val) {
        switch (state) {
            case JRB_BAND_TX_POWER_OFS:
                if (state == JRB_BAND_TX_POWER_OFS) {
                    if (!value->storage.bands.empty()) {
                        if (txPowerOffsetCount >= TX_POWER_OFFSET_MAX_SIZE) {
                            applyErrorDescription("Too many tx offsets");
                            return false;
                        }
                        RegionalParameterChannelPlan &rb(value->storage.bands.back());
                        rb.mut()->txPowerOffsets.push_back((int8_t) val);
                        txPowerOffsetCount++;
                        return true;
                    }
                }
                return false;
            case JRB_BAND_DEFAULTS:
                if (value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                switch (keyIndex) {
                    case JK_RX2FREQUENCY:
                        value->storage.bands.back().mut()->bandDefaults.value.RX2Frequency = (int) val;
                        return true;
                    case JK_RX2DATARATE:
                        value->storage.bands.back().mut()->bandDefaults.value.RX2DataRate = (int) val;
                        return true;
                    case JK_RECEIVEDELAY1:
                        value->storage.bands.back().mut()->bandDefaults.value.ReceiveDelay1 = (int) val;
                        return true;
                    case JK_RECEIVEDELAY2:
                        value->storage.bands.back().mut()->bandDefaults.value.ReceiveDelay2 = (int) val;
                        return true;
                    case JK_JOINACCEPTDELAY1:
                        value->storage.bands.back().mut()->bandDefaults.value.JoinAcceptDelay1 = (int) val;
                        return true;
                    case JK_JOINACCEPTDELAY2:
                        value->storage.bands.back().mut()->bandDefaults.value.JoinAcceptDelay2 = (int) val;
                        return true;
                    default:
                        applyErrorDescription(ERR_REGION_BAND_EMPTY);
                        return false;
                }
            case JRB_BAND_RX1_DR_OFFSET: {
                if (value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                if (rx1DataRateOffsetCount >= DATA_RATE_SIZE) {
                    applyErrorDescription("rx1DataRateOffset more than 8 elements");
                    return false;
                }

                RegionalParameterChannelPlan &rb = value->storage.bands.back();
                rb.mut()->rx1DataRateOffsets[rx1DataRateOffsetCount].push_back((uint8_t) val);
                return true;
            }
            case JRB_BAND_CHANNEL: {
                if (value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                Channel *channel;
                switch (prevState) {
                    case JRB_BAND_UPLINKS:
                        if (value->storage.bands.back().mut()->uplinkChannels.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        channel = &value->storage.bands.back().mut()->uplinkChannels.back();
                        break;
                    case JRB_BAND_DOWNLINKS:
                        if (value->storage.bands.back().mut()->downlinkChannels.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        channel = &value->storage.bands.back().mut()->downlinkChannels.back();
                        break;
                    default:
                        applyErrorDescription(ERR_REGION_BAND_EMPTY);
                        return false;
                }

                switch (keyIndex) {
                    case JK_FREQUENCY:
                        channel->value.frequency = (int) val;
                        return true;
                    case JK_MINDR:
                        channel->value.minDR = (int) val;
                        return true;
                    case JK_MAXDR:
                        channel->value.maxDR = (int) val;
                        return true;
                    default:
                        break;
                }
                applyErrorDescription(ERR_REGION_BAND_EMPTY);
                return false;
            }
            case JRB_BAND_P_SIZE: {
                if (value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                if (maxPayloadSizePerDataRateCount >= DATA_RATE_SIZE) {
                    applyErrorDescription("UInt maxPayloadSizePerDataRate more than 8 elements");
                    // skip error, just warning
                    return true;
                }
                switch (keyIndex) {
                    case JK_M:
                        value->storage.bands.back().mut()->maxPayloadSizePerDataRate[maxPayloadSizePerDataRateCount].value.m = (uint8_t) val;
                        return true;
                    case JK_N:
                        value->storage.bands.back().mut()->maxPayloadSizePerDataRate[maxPayloadSizePerDataRateCount].value.n = (uint8_t) val;
                        return true;
                    default:
                        break;
                }
                applyErrorDescription("Unexpected number in the maxPayloadSizePerDataRate");
                return false;
            }
            case JRB_BAND_PRPT_SIZE: {
                if (value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                if (maxPayloadSizePerDataRateRepeaterCount  >= DATA_RATE_SIZE) {
                    applyErrorDescription("maxPayloadSizePerDataRateRepeater has more than 8 elements");
                    // just warning, not error
                    return true;
                }
                switch (keyIndex) {
                    case JK_M:
                        value->storage.bands.back().mut()->maxPayloadSizePerDataRateRepeater[maxPayloadSizePerDataRateRepeaterCount].value.m = (uint8_t) val;
                        return true;
                    case JK_N:
                        value->storage.bands.back().mut()->maxPayloadSizePerDataRateRepeater[maxPayloadSizePerDataRateRepeaterCount].value.n = (uint8_t) val;
                        return true;
                    default:
                        break;
                }
                applyErrorDescription(ERR_REGION_BAND_EMPTY);
                return false;
            }
            case JRB_BAND_DATA_RATE: {
                switch (keyIndex) {
                    case JK_BANDWIDTH:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        if (dataRateCount > DATA_RATE_SIZE) {
                            applyErrorDescription("dataRates array size bigger than 8 elements");
                            // warning, not error
                            return true;
                        }
                        value->storage.bands.back().mut()->dataRates[dataRateCount - 1].value.bandwidth = int2BANDWIDTH((int) val);
                        return true;
                    case JK_SPREADINGFACTOR:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        if (dataRateCount > DATA_RATE_SIZE) {
                            applyErrorDescription("SF dataRates array size bigger than 8 elements");
                            // warning, not error
                            return true;
                        }
                        value->storage.bands.back().mut()->dataRates[dataRateCount - 1].value.spreadingFactor = (SPREADING_FACTOR) val;
                        return true;
                    case JK_BPS:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        if (dataRateCount > DATA_RATE_SIZE) {
                            applyErrorDescription("BPS dataRates array size bigger than 8 elements");
                            return true;
                        }
                        value->storage.bands.back().mut()->dataRates[dataRateCount - 1].value.bps = (uint32_t) val;
                        return true;
                    default:
                        break;
                }
                applyErrorDescription("Unexpected integer");
                return false;
            }
            case JRB_BAND:
                switch (keyIndex) {
                    case JK_DEFAULTDOWNLINKTXPOWER:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        value->storage.bands.back().mut()->defaultDownlinkTXPower = (int) val;
                        return true;
                    case JK_PINGSLOTFREQUENCY:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        value->storage.bands.back().mut()->pingSlotFrequency = (int) val;
                        return true;
                    case JK_MAXUPLINKEIRP:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        value->storage.bands.back().mut()->maxUplinkEIRP = (float) val;
                        return true;
                    case JK_ID:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        value->storage.bands.back().mut()->id = (uint8_t) val;
                        return true;
                    default:
                        break;
                }
                applyErrorDescription("Unexpected integer value");
                return false;
            default:
                applyErrorDescription("Unexpected integer value");
                return false;
        }
    }

    void applyErrorDescription(const std::string &reason) {
        std::stringstream ss;
        ss << "reason: " << reason << ", key: " << ATTR_REGION_BAND_NAMES[keyIndex] << "(" << keyIndex << ")"
           << ", state: " << PARSE_STATE_NAMES[state];
        value->errDescription = ss.str();
    }

public :
    explicit RegionBandsJsonHandler(RegionalParameterChannelPlanFileJson *val)
        : value(val), keyIndex(JK_NONE), state(JRB_NONE), prevState(JRB_NONE),
        bandCount(0), dataRateCount(0), maxPayloadSizePerDataRateCount(0),
        maxPayloadSizePerDataRateRepeaterCount(0),
        rx1DataRateOffsetCount(0), txPowerOffsetCount(0),
        uplinkChannelCount(0), downlinkChannelCount(0)
    {
    }

    bool null() override {
        return true;
    }

    bool boolean(bool b) override {
        switch (keyIndex) {
            case JK_SUPPORTSEXTRACHANNELS:
                if (value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                value->storage.bands.back().mut()->supportsExtraChannels = b;
                return true;
            case JK_IMPLEMENTSTXPARAMSETUP:
                if (value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                value->storage.bands.back().mut()->implementsTXParamSetup = b;
                return true;
            case JK_DEFAULT_REGION:
                if (value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                value->storage.bands.back().mut()->defaultRegion = b;
                return true;
            case JK_UPLINK:
            case JK_DOWNLINK:
                if (state == JRB_BAND_DATA_RATE) {
                    if (value->storage.bands.empty()) {
                        applyErrorDescription(ERR_REGION_BAND_EMPTY);
                        return false;
                    }
                    if (dataRateCount > DATA_RATE_SIZE) {
                        applyErrorDescription("Downlink dataRates array size bigger than 8 elements");
                        // warning not error
                        return true;
                    }
                    if (keyIndex == JK_UPLINK)
                        value->storage.bands.back().mut()->dataRates[dataRateCount - 1].value.uplink = b;
                    else
                        value->storage.bands.back().mut()->dataRates[dataRateCount - 1].value.downlink = b;
                    return true;
                }
                applyErrorDescription("Unexpected boolean");
                return false;
            case JK_ENABLED:
            case JK_CUSTOM: {
                if (state != JRB_BAND_CHANNEL || value->storage.bands.empty()) {
                    applyErrorDescription(ERR_REGION_BAND_EMPTY);
                    return false;
                }
                switch (prevState) {
                    case JRB_BAND_UPLINKS:
                        if (value->storage.bands.back().mut()->uplinkChannels.empty()) {
                            applyErrorDescription("uplinkChannels array element disappeared ");
                            return false;
                        }
                        break;
                    case JRB_BAND_DOWNLINKS:
                        if (value->storage.bands.back().mut()->downlinkChannels.empty()) {
                            applyErrorDescription("downlinkChannels array element disappeared");
                            return false;
                        }
                        break;
                    default:
                        break;
                }
                Channel &channel = value->storage.bands.back().mut()->uplinkChannels.back();

                switch (keyIndex) {
                    case JK_ENABLED:
                        channel.value.enabled = b;
                        break;
                    case JK_CUSTOM:
                        channel.value.custom = b;
                        break;
                    default:
                            break;
                }
                return true;
            }
            default:
                applyErrorDescription("Unexpected boolean value");
                return false;
        }
    }

    bool number_integer(number_integer_t val) override {
        return Integer(val);
    }

    bool number_unsigned(number_unsigned_t val) override {
        return Integer((number_integer_t) val);
    }

    bool number_float (number_float_t d, const string_t& s) override {
        switch(state) {
            case JRB_BAND:
                switch (keyIndex) {
                    case JK_MAXUPLINKEIRP:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        value->storage.bands.back().mut()->maxUplinkEIRP = (float) d;
                        return true;
                    default:
                        break;
                }
                return false;
            case JRB_BAND_DATA_RATE: {
                switch (keyIndex) {
                    case JK_BANDWIDTH:
                        if (value->storage.bands.empty()) {
                            applyErrorDescription(ERR_REGION_BAND_EMPTY);
                            return false;
                        }
                        if (dataRateCount > DATA_RATE_SIZE) {
                            applyErrorDescription("BW dataRates array size bigger than 8 elements");
                            // warning
                            return true;
                        }
                        value->storage.bands.back().mut()->dataRates[dataRateCount - 1].value.bandwidth = double2BANDWIDTH(d);
                        return true;
                    default:
                        break;
                }
            }
            default:
                break;
        }
        applyErrorDescription("Unexpected float number");
        return false;
    }

    bool string(string_t& str) override {
        switch(keyIndex) {
            case JK_REGIONALPARAMETERSVERSION:
                if (state == JRB_ROOT) {
                    value->storage.regionalParametersVersion = string2REGIONAL_PARAMETERS_VERSION(str);
                    return true;
                }
                applyErrorDescription("String JK_REGIONALPARAMETERSVERSION");
                return false;
            case JK_NAME:
                if (state == JRB_BAND) {
                    if (value->storage.bands.empty()) {
                        applyErrorDescription(ERR_REGION_BAND_EMPTY);
                        return false;
                    }
                    value->storage.bands.back().mut()->name = str;
                    return true;
                }
                applyErrorDescription("String JK_NAME");
                return false;
            case JK_CN:
                if (state == JRB_BAND) {
                    if (value->storage.bands.empty()) {
                        applyErrorDescription(ERR_REGION_BAND_EMPTY);
                        return false;
                    }
                    value->storage.bands.back().mut()->cn = str;
                    return true;
                }
                applyErrorDescription("String JK_CN");
                return false;
            case JK_MODULATION:
                if (state == JRB_BAND_DATA_RATE) {
                    if (value->storage.bands.empty()) {
                        applyErrorDescription(ERR_REGION_BAND_EMPTY);
                        return false;
                    }
                    if (dataRateCount > DATA_RATE_SIZE) {
                        applyErrorDescription("Modulation dataRates array size bigger than 8 elements");
                        // warning
                        return true;
                    }
                    value->storage.bands.back().mut()->dataRates[dataRateCount - 1].value.modulation = string2MODULATION(str.c_str());
                    return true;
                }
                applyErrorDescription("Unexpected boolean");
                return false;
            default:
                return false;
        }
    }

    bool start_object(std::size_t elements) override {
        switch (state) {
            case JRB_NONE:
                state = JRB_ROOT;
                return true;
            case JRB_BANDS:
                state = JRB_BAND;
                {
                    RegionalParameterChannelPlan b;
                    value->storage.bands.push_back(b);
                }
                bandCount++;
                return true;
            case JRB_BAND:
                dataRateCount = 0;
                maxPayloadSizePerDataRateCount = 0;
                maxPayloadSizePerDataRateRepeaterCount = 0;
                rx1DataRateOffsetCount = 0;
                txPowerOffsetCount = 0;
                uplinkChannelCount = 0;
                downlinkChannelCount = 0;
                switch(keyIndex) {
                    case JK_BANDDEFAULTS:
                        state = JRB_BAND_DEFAULTS;
                        return true;
                    default:
                        break;
                }
                applyErrorDescription("Object JRB_BAND JK_BANDDEFAULTS");
                return false;
            case JRB_BAND_DATA_RATES:
                state = JRB_BAND_DATA_RATE;
                dataRateCount++;
                return true;
            case JRB_BAND_P_SIZES:
                state = JRB_BAND_P_SIZE;
                return true;
            case JRB_BAND_PRPT_SIZES:
                state = JRB_BAND_PRPT_SIZE;
                return true;
            case JRB_BAND_UPLINKS:
                state = JRB_BAND_CHANNEL;
                if (!value->storage.bands.empty()) {
                    Channel ch;
                    value->storage.bands.back().mut()->uplinkChannels.push_back(ch);
                    uplinkChannelCount++;
                    return true;
                }
                applyErrorDescription("Object JRB_BAND_UPLINKS");
                return false;
            case JRB_BAND_DOWNLINKS:
                state = JRB_BAND_CHANNEL;
                if (!value->storage.bands.empty()) {
                    Channel ch;
                    value->storage.bands.back().mut()->downlinkChannels.push_back(ch);
                    downlinkChannelCount++;
                    return true;
                }
                applyErrorDescription("Object JRB_BAND_DOWNLINKS");
                return false;
            default:
                applyErrorDescription("Unexpected object");
                return false;
        }
    }

    bool key(string_t& str) override {
        keyIndex = getAttrByName(str.c_str());
        bool ok = keyIndex != JK_NONE;
        if (!ok)
            applyErrorDescription(ERR_REGION_UNKNOWN_ATTRIBUTE + str);
        return ok;
    }

    bool end_object() override {
        switch (state) {
            case JRB_BAND:
                state = JRB_BANDS;
                return true;
            case JRB_ROOT:
                state = JRB_NONE;
                return true;
            case JRB_BAND_DEFAULTS:
                state = JRB_BAND;
                return true;
            case JRB_BAND_DATA_RATE:
                state = JRB_BAND_DATA_RATES;
                return true;
            case JRB_BAND_P_SIZE:
                state = JRB_BAND_P_SIZES;
                maxPayloadSizePerDataRateCount++;
                return true;
            case JRB_BAND_PRPT_SIZE:
                state = JRB_BAND_PRPT_SIZES;
                maxPayloadSizePerDataRateRepeaterCount++;
                return true;
            case JRB_BAND_CHANNEL:
                state = prevState;
                return true;
            default:
                applyErrorDescription("Unexpected end of object");
                return false;
        }
    }

    bool start_array(std::size_t elements) override {
        switch (state) {
            case JRB_ROOT:
                switch (keyIndex) {
                    case JK_REGIONBANDS:
                        state = JRB_BANDS;
                        bandCount = 0;
                        return true;
                    default:
                        applyErrorDescription("Unexpected array");
                        return false;
                }
            case JRB_BAND:
                switch (keyIndex) {
                    case JK_DATA_RATES:
                        state = JRB_BAND_DATA_RATES;
                        return true;
                    case JK_MAXPAYLOADSIZEPERDATARATE:
                        state = JRB_BAND_P_SIZES;
                        return true;
                    case JK_MAXPAYLOADSIZEPERDATARATEREPEATOR:
                        state = JRB_BAND_PRPT_SIZES;
                        return true;
                    case JK_RX1DATARATEOFFSETS:
                        state = JRB_BAND_RX1_DR_OFFSETS;
                        return true;
                    case JK_TXPOWEROFFSETS:
                        state = JRB_BAND_TX_POWER_OFS;
                        return true;
                    case JK_UPLINKCHANNELS:
                        prevState = JRB_BAND_UPLINKS;
                        state = JRB_BAND_UPLINKS;
                        return true;
                    case JK_DOWNLINKCHANNELS:
                        prevState = JRB_BAND_DOWNLINKS;
                        state = JRB_BAND_DOWNLINKS;
                        return true;
                    default:
                        applyErrorDescription("Unexpected array in the band");
                        return false;
                }
            case JRB_BAND_RX1_DR_OFFSETS:
                state = JRB_BAND_RX1_DR_OFFSET;
                return true;
            default:
                applyErrorDescription("Unexpected array");
                return false;
        }
    }

    bool end_array() override {
        switch (state) {
            case JRB_ROOT:
                return true;
            case JRB_BAND:
                state = JRB_BANDS;
                return true;
            case JRB_BANDS:
                state = JRB_ROOT;
                return true;
            case JRB_BAND_DATA_RATES:
            case JRB_BAND_P_SIZES:
            case JRB_BAND_PRPT_SIZES:
            case JRB_BAND_RX1_DR_OFFSETS:
            case JRB_BAND_TX_POWER_OFS:
            case JRB_BAND_UPLINKS:
            case JRB_BAND_DOWNLINKS:
                state = JRB_BAND;
                return true;
            case JRB_BAND_RX1_DR_OFFSET:
                state = JRB_BAND_RX1_DR_OFFSETS;
                rx1DataRateOffsetCount++;
                return true;
            default:
                applyErrorDescription("Unexpected end of array");
                return false;
        }
    }

    bool binary(binary_t& val) override {
        return true;
    }

    bool parse_error(std::size_t position, const std::string& last_token, const nlohmann::json::exception& ex) override {
        std::stringstream ss;
        ss << ERR_REGION_PARSE_ERROR << "[" << position << "] ..." << last_token << "... : " << ex.what();
        applyErrorDescription(ss.str());
        return false;
    }
};

void RegionalParameterChannelPlanFileJson::clear()
{
    storage.bands.clear();
    nameIndex.clear();
    defaultRegionBand = nullptr;
}

/**
 * @return ERR_CODE_REGION_BAND_EMPTY if not loaded any
 */
int RegionalParameterChannelPlanFileJson::buildIndex()
{
    nameIndex.clear();
    idIndex.clear();
    defaultRegionBand = nullptr;
    for (const auto & band : storage.bands) {
        nameIndex[band.get()->name] = &band;
        idIndex[band.get()->id] = &band;
        if (band.get()->defaultRegion)
            defaultRegionBand = &band;
    }
    // Assign default regional settings
    if (!defaultRegionBand) {
        if (storage.bands.empty()) {
            // TODO add dumb region?
        } else {
            // use the first one
            defaultRegionBand = &*storage.bands.begin();
        }
    }
    return storage.bands.empty() ? ERR_CODE_INIT_REGION_NO_DEFAULT : CODE_OK;
}

const RegionalParameterChannelPlan *RegionalParameterChannelPlanFileJson::get(const std::string &name) const
{
    auto it(nameIndex.find(name));
    if (it == nameIndex.end())
        return defaultRegionBand;
    return it->second;
}

const RegionalParameterChannelPlan *RegionalParameterChannelPlanFileJson::get(int id) const
{
    auto it(idIndex.find(id));
    if (it == idIndex.end())
        return defaultRegionBand;
    return it->second;
}

int RegionalParameterChannelPlanFileJson::loadFile(
    const std::string &fileName
)
{
    // parse JSON
    RegionBandsJsonHandler handler(this);
    std::string j = file2string(fileName.c_str());
    auto r = nlohmann::json::sax_parse(j, &handler, nlohmann::json::input_format_t::json, true, true);
    return r ? 0 : ERR_CODE_INVALID_JSON;
}

int RegionalParameterChannelPlanFileJson::load()
{
	clear();
    int r = loadFile(path);
    if (r == 0)
        r = buildIndex();
    return r;
}

int RegionalParameterChannelPlanFileJson::saveFile(const std::string &fileName) const
{
    std::fstream os;
    os.open(fileName.c_str(), std::ios::out);
    os << storage.toString();
    int r = os.bad() ? ERR_CODE_OPEN_DEVICE : 0;
    os.close();
    return r;
}

int RegionalParameterChannelPlanFileJson::save()
{
    int r = saveFile(path);
	return r;
}

int RegionalParameterChannelPlanFileJson::init(
	const std::string &option, 
	void *data
)
{
	path = option;
	return load();
}

void RegionalParameterChannelPlanFileJson::flush()
{
	save();
}

void RegionalParameterChannelPlanFileJson::done()
{

}

std::string RegionalParameterChannelPlanFileJson::toJsonString() const {
    return storage.toString();
}

std::string RegionalParameterChannelPlanFileJson::getErrorDescription(int &subCode) const
{
    subCode = errCode;
    return errDescription;
}

void RegionalParameterChannelPlanFileJson::toHeader(
    std::ostream &strm,
    bool cpp20
) const
{
    bool isFirst = true;
    for (const auto & band : storage.bands) {
        if (isFirst)
            isFirst = false;
        else
            strm << ",\n";
        band.toHeader(strm, 2, cpp20);
    }
}
