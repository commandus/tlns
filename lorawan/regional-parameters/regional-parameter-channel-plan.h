#ifndef REGION_BAND_H_
#define REGION_BAND_H_	1

#include <string>
#include <vector>
#include <cinttypes>
#include "lorawan/lorawan-types.h"

typedef struct RADIO_CHANNEL {
    int frequency;  // frequency in Hz
    int minDR;
    int maxDR;
    bool enabled;
    bool custom;    // this channel was configured by the user
} RADIO_CHANNEL;

// Channel
class Channel : public StringifyIntf {
public:
    RADIO_CHANNEL value;
    Channel();
    Channel(const Channel &value);
    explicit Channel(const RADIO_CHANNEL &value);
    std::string toString() const override;
    void setValue(int frequency, int minDR, int maxDR, bool enabled, bool custom);
};

typedef struct BAND_DEFAULTS {
    // fixed frequency for the RX2 receive window
    int RX2Frequency;
    // fixed data-rate for the RX2 receive window
    int RX2DataRate;
    // RECEIVE_DELAY1 default value
    int ReceiveDelay1;
    // RECEIVE_DELAY2 default value
    int ReceiveDelay2;
    // JOIN_ACCEPT_DELAY1 default value
    int JoinAcceptDelay1;
    // JOIN_ACCEPT_DELAY2 default value.
    int JoinAcceptDelay2;
} BAND_DEFAULTS;

// BandDefaults defines the default bands defined by a band.
class BandDefaults : public StringifyIntf {
public:
    BAND_DEFAULTS value;
    BandDefaults();
    BandDefaults(const BandDefaults &value);
    explicit BandDefaults(const BAND_DEFAULTS &value);
    BandDefaults &operator=(const BandDefaults &value);
    void setValue(
        int RX2Frequency,
        int RX2DataRate,
        int ReceiveDelay1,
        int ReceiveDelay2,
        int JoinAcceptDelay1,
        int JoinAcceptDelay2
    );
    std::string toString() const override;
};

typedef struct MAX_PAYLOAD_SIZE {
    uint8_t m;  // The maximum MACPayload size length
    uint8_t n;  // The maximum application payload length in the absence of the optional FOpt control field
} MAX_PAYLOAD_SIZE;

// MaxPayloadSize defines the max payload size
class MaxPayloadSize : public StringifyIntf {
public:
    MAX_PAYLOAD_SIZE value;
    MaxPayloadSize();
    explicit MaxPayloadSize(const MAX_PAYLOAD_SIZE &value);
    std::string toString() const override;

    void setValue(uint8_t m, uint8_t n);
};

typedef struct REGIONAL_PARAMETER_CHANNEL_PLAN {
    uint8_t id;             // 1..14
    std::string name;       // channel plan name
    std::string cn;         // common name
    float maxUplinkEIRP;    // dBm default
    int defaultDownlinkTXPower; // can depend on frequency
    int pingSlotFrequency;
    bool implementsTXParamSetup;
    bool defaultRegion;     // true- default region
    bool supportsExtraChannels;
    BandDefaults bandDefaults;
    DataRate dataRates[DATA_RATE_SIZE];
    MaxPayloadSize maxPayloadSizePerDataRate[DATA_RATE_SIZE];
    MaxPayloadSize maxPayloadSizePerDataRateRepeater[DATA_RATE_SIZE];    // if repeater is used
    std::vector<uint8_t> rx1DataRateOffsets[DATA_RATE_SIZE];
    // Max EIRP - <offset> dB, 0..16
    std::vector<int8_t> txPowerOffsets;
    std::vector<Channel> uplinkChannels;
    std::vector<Channel> downlinkChannels;
} REGIONAL_PARAMETER_CHANNEL_PLAN;

class RegionalParameterChannelPlan : public StringifyIntf {
public:
    REGIONAL_PARAMETER_CHANNEL_PLAN value;
    // Join Accept delay
    int joinAcceptDelay1() const; // 5s
    int joinAcceptDelay2() const; // 6s

    RegionalParameterChannelPlan();
    explicit RegionalParameterChannelPlan(const REGIONAL_PARAMETER_CHANNEL_PLAN &value);
    RegionalParameterChannelPlan(const RegionalParameterChannelPlan &value);
    std::string toString() const override;

    void setTxPowerOffsets(int count, ...);

    void setRx1DataRateOffsets(int dataRateIndex, int count, ...);

    std::string toDescriptionTableString() const;
    void toHeader(std::ostream &strm, int tabs) const;
};

class RegionBands : public StringifyIntf {
public:
    REGIONAL_PARAMETERS_VERSION regionalParametersVersion;  // since specified LoraWAN regional parameters version, if version 0.0.0- any(default) version
    std::vector<RegionalParameterChannelPlan> bands;
    RegionBands();
    RegionBands(const RegionBands &value);
    explicit RegionBands(const std::vector<REGIONAL_PARAMETER_CHANNEL_PLAN> &bands);
    const RegionalParameterChannelPlan* get(const std::string &name) const;
    std::string toString() const override;
    bool setRegionalParametersVersion(const std::string &value);
};

#endif
