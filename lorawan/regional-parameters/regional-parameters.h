/**
 * This file intended for reference only.
 *
 * Regional parameters control types copied from hhe LoRaMac-node open source code
 * Revised BSD License
 * Copyright Semtech Corporation 2013. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the Semtech corporation nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * HIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string>
#include <cinttypes>
#include <vector>

#define DR_0    0

#define DR_5    5

enum RPPhyAttribute {
    PHY_FREQUENCY,             ///< Frequency. It is available to perform a verification with RegionVerify()
    PHY_MIN_RX_DR,             ///< Minimum RX datarate
    PHY_MIN_TX_DR,             ///< Minimum TX datarate
    PHY_MAX_RX_DR,             ///< Maximum RX datarate
    PHY_MAX_TX_DR,             ///< Maximum TX datarate
    PHY_TX_DR,                 ///< TX datarate. This is a parameter which can't be queried. It is available to perform a verification with RegionVerify().
    PHY_DEF_TX_DR,             ///< Default TX datarate.
    PHY_RX_DR,                 ///< RX datarate. It is available to perform a verification with RegionVerify().
    PHY_MAX_TX_POWER,          ///< Maximum TX power
    PHY_TX_POWER,              ///< TX power. It is available to perform a verification with RegionVerify()
    PHY_DEF_TX_POWER,          ///< Default TX power
    PHY_DEF_ADR_ACK_LIMIT,     ///< Default ADR_ACK_LIMIT value
    PHY_DEF_ADR_ACK_DELAY,     ///< Default ADR_ACK_DELAY value
    PHY_MAX_PAYLOAD,           ///< Maximum payload possible
    PHY_DUTY_CYCLE,            ///< Duty cycle
    PHY_MAX_RX_WINDOW,         ///< Maximum receive window duration
    PHY_RECEIVE_DELAY1,        ///< Receive delay for window 1
    PHY_RECEIVE_DELAY2,        ///< Receive delay for window 2
    PHY_JOIN_ACCEPT_DELAY1,    ///< Join accept delay for window 1
    PHY_JOIN_ACCEPT_DELAY2,    ///< Join accept delay for window 2
    PHY_RETRANSMIT_TIMEOUT,    ///< Acknowledgement time out
    PHY_DEF_DR1_OFFSET,        ///< Default datarate offset for window 1
    PHY_DEF_RX2_FREQUENCY,     ///< Default receive window 2 frequency
    PHY_DEF_RX2_DR,            ///< Default receive window 2 datarate
    PHY_CHANNELS_MASK,         ///< Channels mask
    PHY_CHANNELS_DEFAULT_MASK, ///< Channels default mask
    PHY_MAX_NB_CHANNELS,       ///< Maximum number of supported channels
    PHY_CHANNELS,              ///< Channels
    PHY_DEF_UPLINK_DWELL_TIME, ///< Default value of the uplink dwell time
    PHY_DEF_DOWNLINK_DWELL_TIME, ///< Default value of the downlink dwell time
    PHY_DEF_MAX_EIRP,          ///< Default value of the MaxEIRP
    PHY_DEF_ANTENNA_GAIN,      ///< Default value of the antenna gain
    PHY_NEXT_LOWER_TX_DR,      ///< Next lower datarate
    PHY_BEACON_INTERVAL,       ///< Beacon interval in ms
    PHY_BEACON_RESERVED,       ///< Beacon reserved time in ms
    PHY_BEACON_GUARD,          ///< Beacon guard time in ms
    PHY_BEACON_WINDOW,         ///< Beacon window time in ms
    PHY_BEACON_WINDOW_SLOTS,   ///< Beacon window time in numer of slots
    PHY_PING_SLOT_WINDOW,      ///< Ping slot length time in ms
    PHY_BEACON_SYMBOL_TO_DEFAULT, ///< Default symbol timeout for beacons and ping slot windows
    PHY_BEACON_SYMBOL_TO_EXPANSION_MAX, ///< Maximum symbol timeout for beacons
    PHY_PING_SLOT_SYMBOL_TO_EXPANSION_MAX, ///< Maximum symbol timeout for ping slots
    PHY_BEACON_SYMBOL_TO_EXPANSION_FACTOR, ///< Symbol expansion value for beacon windows in case of beacon loss in symbols.
    PHY_PING_SLOT_SYMBOL_TO_EXPANSION_FACTOR, ///< Symbol expansion value for ping slot windows in case of beacon loss in symbols.
    PHY_MAX_BEACON_LESS_PERIOD, ///< Maximum allowed beacon less time in ms
    PHY_BEACON_DELAY_BEACON_TIMING_ANS, ///< Delay time for the BeaconTimingAns in ms
    PHY_BEACON_CHANNEL_FREQ,    ///< Beacon channel frequency
    PHY_BEACON_FORMAT,          ///< The format of the beacon
    PHY_BEACON_CHANNEL_DR,      ///< The beacon channel datarate
    PHY_BEACON_NB_CHANNELS,     ///< The number of channels for the beacon reception
    PHY_BEACON_CHANNEL_OFFSET,  ///< The static offset for the downlink channel calculation
    PHY_PING_SLOT_CHANNEL_FREQ, ///< Ping slot channel frequency
    PHY_PING_SLOT_CHANNEL_DR,   ///< The datarate of a ping slot channel
    PHY_PING_SLOT_NB_CHANNELS,  ///< The number of channels for the ping slot reception
    PHY_SF_FROM_DR,             ///< The equivalent spreading factor value from datarate
    PHY_BW_FROM_DR              ///< The equivalent bandwith index from datarate
};

enum RPInitType {
    INIT_TYPE_DEFAULTS,         ///< Initializes the regional default settings for the band, channel and default channels mask. Some regions also initiate other default configurations. In general, this type is intended to be called once during the initialization.
    INIT_TYPE_RESET_TO_DEFAULT_CHANNELS, ///< Resets the channels mask to the default channels. Deactivates all other channels.
    INIT_TYPE_ACTIVATE_DEFAULT_CHANNELS ///< Activates the default channels. Leaves all other active channels active.
};

typedef struct {
    RPPhyAttribute Attribute;    ///< Setup the parameter to get
    int8_t Datarate;             ///< Datarate. The parameter is needed for the following queries: PHY_MAX_PAYLOAD, PHY_NEXT_LOWER_TX_DR, PHY_SF_FROM_DR, PHY_BW_FROM_DR.
    uint8_t UplinkDwellTime;     ///<Uplink dwell time. This parameter must be set to query: PHY_MAX_PAYLOAD, PHY_MIN_TX_DR. The parameter is needed for the following queries: PHY_MIN_TX_DR, PHY_MAX_PAYLOAD, PHY_NEXT_LOWER_TX_DR.
    uint8_t DownlinkDwellTime;   ///< Downlink dwell time. This parameter must be set to query: PHY_MAX_PAYLOAD, PHY_MIN_RX_DR. The parameter is needed for the following queries: PHY_MIN_RX_DR, PHY_MAX_PAYLOAD.
    uint8_t Channel;             ///< Specification of the downlink channel. Used in Class B only. The parameter is needed for the following queries: PHY_BEACON_CHANNEL_FREQ, PHY_PING_SLOT_CHANNEL_FREQ
} RPGetPhyParams;

typedef struct {
    uint8_t BeaconSize;          ///< Size of the beacon
    uint8_t Rfu1Size;            ///< Size of the RFU 1 data field
    uint8_t Rfu2Size;            ///< Size of the RFU 2 data field
} RPBeaconFormat;

typedef union {
    uint32_t Frequency;          ///< Channel frequency to verify
    int8_t TxPower;              ///< TX power to verify
    bool DutyCycle;              ///< Set to true, if the duty cycle is enabled, otherwise false.
    struct {                     ///< Datarate to verify
        int8_t Datarate;         ///< Datarate to verify
        uint8_t DownlinkDwellTime; ///< The downlink dwell time
        uint8_t UplinkDwellTime; ///< The up link dwell time.
   } DatarateParams;
} RPVerifyParams;

typedef struct {
    uint32_t Frequency;           ///< Frequency in Hz
    uint8_t  Datarate;            ///< Data rate LoRaWAN Regional Parameters V1.0.2rB The allowed ranges are region specific. Please refer to \ref DR_0 to \ref DR_15 for details.
} RPRxChannelParams;

typedef union {
    int8_t Value;                 ///< Byte-access to the bits
    struct sFields {              ///< Structure to store the minimum and the maximum datarate
        int8_t Min : 4;           ///< Minimum data rate LoRaWAN Regional Parameters V1.0.2rB The allowed ranges are region specific. Please refer to \ref DR_0 to \ref DR_15 for details.
        int8_t Max : 4;           ///< Maximum data rate LoRaWAN Regional Parameters V1.0.2rB The allowed ranges are region specific. Please refer to \ref DR_0 to \ref DR_15 for details.
    } Fields;
} RPDrRange;

typedef struct sChannelParams {
    uint32_t Frequency;           ///< Frequency in Hz
    uint32_t Rx1Frequency;        ///< Alternative frequency for RX window 1
    RPDrRange DrRange;            ///< Data rate definition
    uint8_t Band;                 ///< Band index
} RPChannelParams;

typedef union {
    uint32_t Value;                 ///< A parameter value
    float fValue;                   ///< A floating point value
    uint16_t* ChannelsMask;         ///< Pointer to the channels mask
    RPChannelParams* Channels;      ///< Pointer to the channels
    RPBeaconFormat BeaconFormat;    ///< Beacon format
    uint32_t DutyCycleTimePeriod;   ///< Duty Cycle Period
} RPPhyParam;

typedef struct {
    uint32_t Seconds;               ///< s
    int16_t  SubSeconds;            ///< ms
} RPSysTime;

typedef struct sSetBandTxDoneParams {
    uint8_t Channel;                ///< Channel to update
    bool Joined;                    ///< Joined Set to true, if the node has joined the network
    uint32_t LastTxDoneTime;        ///< Last TX done time
    uint32_t LastTxAirTime;         ///< Time-on-air of the last transmission
    RPSysTime ElapsedTimeSinceStartUp; ///< Elapsed time since initialization
} RPSetBandTxDoneParams;

typedef struct {
    void* NvmGroup1;             ///< Pointer to region NVM group1
    void* NvmGroup2;             ///< Pointer to region NVM group2.
    void* Bands;                 ///< Pointer to common region band storage.
    RPInitType Type;             ///< Sets the initialization type.
} RPInitDefaultsParams;

typedef struct {
    uint8_t JoinChannel;
    uint8_t* Payload;           ///< Payload which contains the CF list
    uint8_t Size;               ///< Size of the payload
} RPApplyCFListParams;

enum RPChannelsMask {
    CHANNELS_MASK,              ///< The channels mask
    CHANNELS_DEFAULT_MASK       ///< The channels default mask
};

typedef struct {
    uint16_t* ChannelsMaskIn;    ///< Pointer to the channels mask which should be set
    RPChannelsMask ChannelsMaskType; ///< Pointer to the channels mask which should be set
} RPChanMaskSetParams;

enum RPLoRaMacRxSlot {
    RX_SLOT_WIN_1,               ///< LoRaMAC receive window 1
    RX_SLOT_WIN_2,               ///< LoRaMAC receive window 2
    RX_SLOT_WIN_CLASS_C,         ///< LoRaMAC receive window 2 for class C - continuous listening
    RX_SLOT_WIN_CLASS_C_MULTICAST, ///< LoRaMAC class C multicast downlink
    RX_SLOT_WIN_CLASS_B_PING_SLOT, ///< LoRaMAC class B ping slot window
    RX_SLOT_WIN_CLASS_B_MULTICAST_SLOT, ///< LoRaMAC class b multicast slot window
    RX_SLOT_NONE                 ///< LoRaMAC no active receive window
};

enum RPActivationType {
    ACTIVATION_TYPE_NONE = 0,
    ACTIVATION_TYPE_ABP = 1,     ///< Activation By Personalization
    ACTIVATION_TYPE_OTAA = 2     ///< Over-The-Air Activation
};

typedef struct {
    uint8_t Channel;             ///< The RX channel
    int8_t Datarate;             ///< RX datarate
    uint8_t Bandwidth;           ///< RX bandwidth
    int8_t DrOffset;             ///< RX datarate offset
    uint32_t Frequency;          ///< RX frequency
    uint32_t WindowTimeout;      ///< RX window timeout
    int32_t WindowOffset;        ///< RX window offset
    uint8_t DownlinkDwellTime;   ///< Downlink dwell time
    bool RxContinuous;           ///< Set to true, if RX should be continuous
    RPLoRaMacRxSlot RxSlot;      ///< Sets the RX window
    RPActivationType NetworkActivation; ///< LoRaWAN Network End-Device Activation ( ACTIVATION_TYPE_NONE, ACTIVATION_TYPE_ABP or ACTIVATION_TYPE_OTTA)
} RPRxConfigParams;

typedef struct {
    uint8_t Channel;             ///< The TX channel
    int8_t Datarate;             ///< The TX datarate
    int8_t TxPower;              ///< The TX power
    float MaxEirp;               ///< The Max EIRP, if applicable
    float AntennaGain;           ///< The antenna gain, if applicable
    uint16_t PktLen;             ///< Frame length to setup
} RPTxConfigParams;

typedef union {
    struct {
        uint8_t Revision;
        uint8_t Patch;
        uint8_t Minor;
        uint8_t Major;
    } Fields;
    uint32_t Value;
} RPVersion;

typedef struct {
    RPVersion Version;              ///< Current LoRaWAN Version
    uint8_t* Payload;               ///< Pointer to the payload which contains the MAC commands
    uint8_t PayloadSize;            ///< Size of the payload
    uint8_t UplinkDwellTime;        ///< Uplink dwell time
    bool AdrEnabled;                ///< Set to true, if ADR is enabled
    int8_t CurrentDatarate;         ///< The current datarate
    int8_t CurrentTxPower;          ///< The current TX power
    uint8_t CurrentNbRep;           ///< The current number of repetitions
} RPLinkAdrReqParams;

typedef struct {
    int8_t Datarate;                ///< The datarate to setup
    int8_t DrOffset;                ///< Datarate offset
    uint32_t Frequency;             ///< The frequency to setup
} RPRxParamSetupReqParams;

typedef struct {
    uint8_t UplinkDwellTime;        ///< Uplink dwell time
    uint8_t DownlinkDwellTime;      ///< Downlink dwell time
    uint8_t MaxEirp;                ///< Max EIRP
} RPTxParamSetupReqParams;

typedef struct {
    RPChannelParams* NewChannel;    ///< Pointer to the new channels
    int8_t ChannelId;               ///< Channel id
} RPNewChannelReqParams;

typedef struct {
    uint8_t ChannelId;              ///< Channel Id to add the frequency
    uint32_t Rx1Frequency;          ///< Alternative frequency for the Rx1 window
} RPDlChannelReqParams;

enum RPAlternateDrType {
    ALTERNATE_DR,                   ///< Type to use for an alternation
    ALTERNATE_DR_RESTORE            ///< Type to use to restore one alternation
};

typedef struct {
    uint32_t AggrTimeOff;           ///< Aggregated time-off time
    uint32_t LastAggrTx;            ///< Time of the last aggregated TX
    int8_t Datarate;                ///< Current datarate
    bool Joined;                    ///< Set to true, if the node has already joined a network, otherwise false
    bool DutyCycleEnabled;          ///< Set to true, if the duty cycle is enabled, otherwise false
    RPSysTime ElapsedTimeSinceStartUp;  ///< Elapsed time since the start of the node
    bool LastTxIsJoinRequest;       ///< Joined Set to true, if the last uplink was a join request
    uint16_t PktLen;                ///< Payload length of the next frame
} RPNextChanParams;

typedef struct {
    RPChannelParams* NewChannel;    ///< Pointer to the new channel to add
    uint8_t ChannelId;              ///< Channel id to add
} RPChannelAddParams;

typedef struct {
    uint8_t ChannelId;               ///< Channel id to remove
} RPChannelRemoveParams;

typedef struct {
    uint16_t SymbolTimeout;          ///< Symbol timeout
    uint32_t RxTime;                 ///< Receive time
    uint32_t Frequency;              ///< The frequency to setup
} RPRxBeaconSetup;

typedef struct RPBand {
    uint16_t DCycle;                    ///< Duty cycle
    int8_t TxMaxPower;                  ///< Maximum Tx power
    uint32_t LastBandUpdateTime;        ///< The last time the band has been synchronized with the current time
    uint32_t LastMaxCreditAssignTime;   ///< The last time we have assigned the max credits for the 24h interval.
    uint32_t TimeCredits;               ///< Current time credits which are available. This is a value in ms
    uint32_t MaxTimeCredits;            ///< Maximum time credits which are available. This is a value in ms
    bool ReadyForTransmission;          ///< Set to true when the band is ready for use

    RPBand& operator=(const RPBand& other) {
        if (this != &other) { // Self-assignment check
            DCycle = other.DCycle;
            TxMaxPower = other.TxMaxPower;
            LastBandUpdateTime = other.LastBandUpdateTime;
            LastMaxCreditAssignTime = other.LastMaxCreditAssignTime;
            TimeCredits = other.TimeCredits;
            MaxTimeCredits = other.MaxTimeCredits;
            ReadyForTransmission = other.ReadyForTransmission;
        }
        return *this;
    }
} RPBand;

#define REGION_MAX_CHANNELS                 96
#define REGION_MAX_MASK                     6

class RPRegionNvmDataGroup2 {
public:
    std::vector<RPChannelParams> Channels;          ///< LoRaMAC channels CN470: 96, US915, AU915: 72, others: 16
    std::vector<uint16_t> ChannelsMask;             ///< LoRaMac channels mask CN470, US915, AU915: 6, others: 1
    std::vector<uint16_t> ChannelsDefaultMask;      ///< LoRaMac channels default mask
    uint32_t Crc32;                                 ///< CRC32 value of the Region data structure
};

typedef struct {
    std::vector<uint8_t> Datarates;                 ///< Available datarates
    uint32_t Frequency;                             ///< Frequency
    uint8_t BeaconSize;                             ///< The size of the beacon frame
    uint8_t BeaconDatarate;                         ///< datarate of the beacon
    uint8_t BeaconChannelBW;                        ///< The channel bandwidth of the beacon
    uint32_t RxTime;                                ///< RX time
    uint16_t SymbolTimeout;                         ///< The symbol timeout of the RX procedure
} RPRegionCommonRxBeaconSetupParams;

/**
 * @brief Sets the last tx done property. This is a generic function and valid for all regions.
 * @param [IN] band The band to be updated.
 * @param [IN] lastTxAirTime The time on air of the last TX frame.
 * @param [IN] joined Set to true if the device has joined.
 * @param [IN] elapsedTimeSinceStartup Elapsed time since initialization.
 */
void RegionCommonSetBandTxDone(RPBand* band, uint32_t lastTxAirTime, bool joined, RPSysTime elapsedTimeSinceStartup);

/**
 * @brief Disables a channel in a given channels mask. This is a generic function and valid for all regions.
 * @param [IN] channelsMask The channels mask of the region.
 * @param [IN] id The id of the channels mask to disable.
 * @param [IN] maxChannels Maximum number of channels.
 * @retval Returns true if the channel could be disabled, false if not.
 */
bool RegionCommonChanDisable(std::vector <uint16_t> &channelsMask, uint8_t id, uint8_t maxChannels);

uint16_t GetDutyCycle(
    RPBand* band,
    bool joined,
    RPSysTime elapsedTimeSinceStartup
);

bool RegionCommonValueInRange(
    int8_t value,
    int8_t min,
    int8_t max
);

void RegionCommonRxBeaconSetup(
    RPRegionCommonRxBeaconSetupParams* rxBeaconSetupParams
);

class RegionalParameters {
public:
    RPRegionNvmDataGroup2* RegionNvmGroup2;  ///< Region independent non-volatile data module context 2
    std::vector<RPBand> RegionBands;

    RegionalParameters();
    RegionalParameters(const RegionalParameters &value);
    RegionalParameters(uint8_t id);
    std::string toString() const;
    void setValue(int frequency, int minDR, int maxDR, bool enabled, bool custom);

    /**
    * @brief Verifies if a region is active or not. If a region is not active, it cannot be used.
    * @retval Return true, if the region is supported.
    */
    static bool RegionIsActive();

    /**
     * @brief The function gets a value of a specific phy attribute.
     * @param getPhy Pointer to the function parameters.
     * @retval Returns a structure containing the PHY parameter.
     */
    virtual RPPhyParam RegionGetPhyParam(RPGetPhyParams* getPhy);

    /**
     * @brief Updates the last TX done parameters of the current channel.
     * @param [IN] txDone Pointer to the function parameters.
     */
    virtual void RegionSetBandTxDone(RPSetBandTxDoneParams* txDone);

    /**
     * @brief Initializes the channels masks and the channels.
     * @param [IN] params Pointer to the function parameters.
     */
    virtual void RegionInitDefaults(RPInitDefaultsParams* params);

    /**
     * @brief Verifies a parameter.
     * @param [IN] verify Pointer to the function parameters.
     * @param [IN] type Sets the initialization type.
     * @retval Returns true, if the parameter is valid.
     */
    virtual bool RegionVerify(RPVerifyParams* verify, RPPhyAttribute phyAttribute);

    /**
     * @brief The function parses the input buffer and sets up the channels of the CF list.
     * @param [IN] applyCFList Pointer to the function parameters.
     */
    virtual void RegionApplyCFList(RPApplyCFListParams* applyCFList);

    /**
     * @brief Sets a channels mask.
     * @param [IN] chanMaskSet Pointer to the function parameters.
     * @retval Returns true, if the channels mask could be set.
     */
    virtual bool RegionChanMaskSet(RPChanMaskSetParams* chanMaskSet);

    /**
     * @brief Configuration of the RX windows.
     * @param [IN] rxConfig Pointer to the function parameters.
     * @param [OUT] datarate The datarate index which was set.
     * @retval Returns true, if the configuration was applied successfully.
     */
    virtual bool RegionRxConfig(RPRxConfigParams* rxConfig, int8_t* datarate);

    /*
    * Rx window precise timing
    *
    * For more details please consult the following document, chapter 3.1.2.
    * https://www.semtech.com/uploads/documents/SX1272_settings_for_LoRaWAN_v2.0.pdf
    * or
    * https://www.semtech.com/uploads/documents/SX1276_settings_for_LoRaWAN_v2.0.pdf
    *
    *                 Downlink start: T = Tx + 1s (+/- 20 us)
    *                            |
    *             TRxEarly       |        TRxLate
    *                |           |           |
    *                |           |           +---+---+---+---+---+---+---+---+
    *                |           |           |       Latest Rx window        |
    *                |           |           +---+---+---+---+---+---+---+---+
    *                |           |           |
    *                +---+---+---+---+---+---+---+---+
    *                |       Earliest Rx window      |
    *                +---+---+---+---+---+---+---+---+
    *                            |
    *                            +---+---+---+---+---+---+---+---+
    *Downlink preamble 8 symbols |   |   |   |   |   |   |   |   |
    *                            +---+---+---+---+---+---+---+---+
    *
    *                     Worst case Rx window timings
    *
    * TRxLate  = DEFAULT_MIN_RX_SYMBOLS * tSymbol - RADIO_WAKEUP_TIME
    * TRxEarly = 8 - DEFAULT_MIN_RX_SYMBOLS * tSymbol - RxWindowTimeout - RADIO_WAKEUP_TIME
    *
    * TRxLate - TRxEarly = 2 * DEFAULT_SYSTEM_MAX_RX_ERROR
    *
    * RxOffset = (TRxLate + TRxEarly) / 2
    *
    * RxWindowTimeout = (2 * DEFAULT_MIN_RX_SYMBOLS - 8) * tSymbol + 2 * DEFAULT_SYSTEM_MAX_RX_ERROR
    * RxOffset = 4 * tSymbol - RxWindowTimeout / 2 - RADIO_WAKE_UP_TIME
    *
    * Minimal value of RxWindowTimeout must be 5 symbols which implies that the system always tolerates at least an error of 1.5 * tSymbol
    */
    /**
     * Computes the Rx window timeout and offset.
     * @param [IN] datarate     Rx window datarate index to be used
     * @param [IN] minRxSymbols Minimum required number of symbols to detect an Rx frame.
     * @param [IN] rxError      System maximum timing error of the receiver. In milliseconds
     *                          The receiver will turn on in a [-rxError : +rxError] ms
     *                          interval around RxOffset
     * @param [OUT]rxConfigParams Returns updated WindowTimeout and WindowOffset fields.
     */
    virtual void RegionComputeRxWindowParameters(int8_t datarate, uint8_t minRxSymbols, uint32_t rxError, RPRxConfigParams *rxConfigParams);

    /**
     * @brief TX configuration.
     * @param [IN] txConfig Pointer to the function parameters.
     * @param [OUT] txPower The tx power index which was set.
     * @param [OUT] txTimeOnAir The time-on-air of the frame.
     * @retval Returns true, if the configuration was applied successfully.
     */
    virtual bool RegionTxConfig(RPTxConfigParams* txConfig, int8_t* txPower, uint32_t* txTimeOnAir);

    /**
     * @brief The function processes a Link ADR Request.
     * @param [IN] linkAdrReq Pointer to the function parameters.
     * @param [OUT] drOut The datarate which was applied.
     * @param [OUT] txPowOut The TX power which was applied.
     * @param [OUT] nbRepOut The number of repetitions to apply.
     * @param [OUT] nbBytesParsed The number bytes which were parsed.
     * @retval Returns the status of the operation, according to the LoRaMAC specification.
     */
    virtual uint8_t RegionLinkAdrReq(RPLinkAdrReqParams* linkAdrReq, int8_t* drOut, int8_t* txPowOut, uint8_t* nbRepOut, uint8_t* nbBytesParsed);

    /**
     * @brief The function processes a RX Parameter Setup Request.
     * @param [IN] rxParamSetupReq Pointer to the function parameters.
     * @retval Returns the status of the operation, according to the LoRaMAC specification.
     */
    virtual uint8_t RegionRxParamSetupReq(RPRxParamSetupReqParams* rxParamSetupReq);

    /**
     * @brief The function processes a New Channel Request.
     * @param [IN] newChannelReq Pointer to the function parameters.
     * @retval Returns the status of the operation, according to the LoRaMAC specification.
     */
    virtual int8_t RegionNewChannelReq(RPNewChannelReqParams* newChannelReq);

    /**
     * @brief The function processes a TX ParamSetup Request.
     * @param [IN] txParamSetupReq Pointer to the function parameters.
     * @retval Returns the status of the operation, according to the LoRaMAC specification.
     *         Returns -1, if the functionality is not implemented. In this case, the end node
     *         shall ignore the command.
     */
    virtual int8_t RegionTxParamSetupReq(RPTxParamSetupReqParams* txParamSetupReq);

    /**
     * @brief The function processes a DlChannel Request.
     * @param [IN] dlChannelReq Pointer to the function parameters.
     * @retval Returns the status of the operation, according to the LoRaMAC specification.
     */
    virtual int8_t RegionDlChannelReq(RPDlChannelReqParams* dlChannelReq);

    /**
     * @brief Alternates the datarate of the channel for the join request.
     * @param [IN] currentDr Current datarate.
     * @param [IN] type Alternation type.
     * @retval Datarate to apply.
     */
    virtual int8_t RegionAlternateDr(int8_t currentDr, RPAlternateDrType type);

    /**
     * @brief Searches and set the next random available channel
     * @param [OUT] channel Next channel to use for TX.
     * @param [OUT] time Time to wait for the next transmission according to the duty
     *              cycle.
     * @param [OUT] aggregatedTimeOff Updates the aggregated time off.
     * @retval Function status [true: OK, false: Unable to find a channel on the current datarate].
     */
    virtual bool RegionNextChannel(RPNextChanParams* nextChanParams, uint8_t* channel, uint32_t* time, uint32_t* aggregatedTimeOff);

    /**
     * @brief Adds a channel.
     * @param [IN] channelAdd Pointer to the function parameters.
     * @retval Status of the operation.
     */
    virtual bool RegionChannelAdd(RPChannelAddParams* channelAdd);

    /**
     * @brief Removes a channel.
     * @param [IN] channelRemove Pointer to the function parameters.
     * @retval Returns true, if the channel was removed successfully.
     */
    virtual bool RegionChannelsRemove(RPChannelRemoveParams* channelRemove);

    /**
     * @brief Computes new datarate according to the given offset
     * @param [IN] downlinkDwellTime Downlink dwell time configuration. 0: No limit, 1: 400ms
     * @param [IN] dr Current datarate
     * @param [IN] drOffset Offset to be applied
     * @retval newDr Computed datarate.
     */
    virtual uint8_t RegionApplyDrOffset(uint8_t downlinkDwellTime, int8_t dr, int8_t drOffset);

    /**
     * @brief Sets the radio into beacon reception mode
     * @param [IN] rxBeaconSetup Pointer to the function parameters
     * @param [out] outDr Datarate used to receive the beacon
     */
    virtual void RegionRxBeaconSetup(RPRxBeaconSetup* rxBeaconSetup, uint8_t* outDr);
};

class RegionalParametersCollection {
public:
    RegionalParameters* value[256];

    RegionalParametersCollection();
    RegionalParametersCollection(const RegionalParametersCollection &value);
    std::string toString() const;

    /**
    * @brief Verifies if a region is active or not. If a region is not active, it cannot be used.
    * @param region LoRaWAN region.
    * @retval Return true, if the region is supported.
    */
    bool RegionIsActive(uint8_t region);
    /**
     * @brief The function gets a value of a specific phy attribute.
     * @param region LoRaWAN region.
     * @param getPhy Pointer to the function parameters.
     * @retval Returns a structure containing the PHY parameter.
     */
    RPPhyParam RegionGetPhyParam(uint8_t region, RPGetPhyParams* getPhy);

    /**
     * @brief Updates the last TX done parameters of the current channel.
     * @param [IN] region LoRaWAN region.
     * @param [IN] txDone Pointer to the function parameters.
     */
    void RegionSetBandTxDone(uint8_t region, RPSetBandTxDoneParams* txDone);

    /**
     * @brief Initializes the channels masks and the channels.
     * @param [IN] region LoRaWAN region.
     * @param [IN] params Pointer to the function parameters.
     */
    void RegionInitDefaults(uint8_t region, RPInitDefaultsParams* params);

    /**
     * @brief Verifies a parameter.
     * @param [IN] region LoRaWAN region.
     * @param [IN] verify Pointer to the function parameters.
     * @param [IN] type Sets the initialization type.
     * @retval Returns true, if the parameter is valid.
     */
    bool RegionVerify(uint8_t region, RPVerifyParams* verify, RPPhyAttribute phyAttribute);

    /**
     * @brief The function parses the input buffer and sets up the channels of the
     *        CF list.
     *
     * @param [IN] region LoRaWAN region.
     *
     * @param [IN] applyCFList Pointer to the function parameters.
     */
    void RegionApplyCFList(uint8_t region, RPApplyCFListParams* applyCFList);

    /**
     * @brief Sets a channels mask.
     * @param [IN] region LoRaWAN region.
     * @param [IN] chanMaskSet Pointer to the function parameters.
     * @retval Returns true, if the channels mask could be set.
     */
    bool RegionChanMaskSet(uint8_t region, RPChanMaskSetParams* chanMaskSet);

    /**
     * @brief Configuration of the RX windows.
     * @param [IN] region LoRaWAN region.
     * @param [IN] rxConfig Pointer to the function parameters.
     * @param [OUT] datarate The datarate index which was set.
     * @retval Returns true, if the configuration was applied successfully.
     */
    bool RegionRxConfig(uint8_t region, RPRxConfigParams* rxConfig, int8_t* datarate);

    /*
    * Rx window precise timing
    *
    * For more details please consult the following document, chapter 3.1.2.
    * https://www.semtech.com/uploads/documents/SX1272_settings_for_LoRaWAN_v2.0.pdf
    * or
    * https://www.semtech.com/uploads/documents/SX1276_settings_for_LoRaWAN_v2.0.pdf
    *
    *                 Downlink start: T = Tx + 1s (+/- 20 us)
    *                            |
    *             TRxEarly       |        TRxLate
    *                |           |           |
    *                |           |           +---+---+---+---+---+---+---+---+
    *                |           |           |       Latest Rx window        |
    *                |           |           +---+---+---+---+---+---+---+---+
    *                |           |           |
    *                +---+---+---+---+---+---+---+---+
    *                |       Earliest Rx window      |
    *                +---+---+---+---+---+---+---+---+
    *                            |
    *                            +---+---+---+---+---+---+---+---+
    *Downlink preamble 8 symbols |   |   |   |   |   |   |   |   |
    *                            +---+---+---+---+---+---+---+---+
    *
    *                     Worst case Rx window timings
    *
    * TRxLate  = DEFAULT_MIN_RX_SYMBOLS * tSymbol - RADIO_WAKEUP_TIME
    * TRxEarly = 8 - DEFAULT_MIN_RX_SYMBOLS * tSymbol - RxWindowTimeout - RADIO_WAKEUP_TIME
    *
    * TRxLate - TRxEarly = 2 * DEFAULT_SYSTEM_MAX_RX_ERROR
    *
    * RxOffset = (TRxLate + TRxEarly) / 2
    *
    * RxWindowTimeout = (2 * DEFAULT_MIN_RX_SYMBOLS - 8) * tSymbol + 2 * DEFAULT_SYSTEM_MAX_RX_ERROR
    * RxOffset = 4 * tSymbol - RxWindowTimeout / 2 - RADIO_WAKE_UP_TIME
    *
    * Minimal value of RxWindowTimeout must be 5 symbols which implies that the system always tolerates at least an error of 1.5 * tSymbol
    */
    /**
     * Computes the Rx window timeout and offset.
     * @param [IN] region       LoRaWAN region.
     * @param [IN] datarate     Rx window datarate index to be used
     * @param [IN] minRxSymbols Minimum required number of symbols to detect an Rx frame.
     * @param [IN] rxError      System maximum timing error of the receiver. In milliseconds
     *                          The receiver will turn on in a [-rxError : +rxError] ms
     *                          interval around RxOffset
     * @param [OUT]rxConfigParams Returns updated WindowTimeout and WindowOffset fields.
     */
    void RegionComputeRxWindowParameters(uint8_t region, int8_t datarate, uint8_t minRxSymbols, uint32_t rxError, RPRxConfigParams *rxConfigParams);

    /**
     * @brief TX configuration.
     * @param [IN] region LoRaWAN region.
     * @param [IN] txConfig Pointer to the function parameters.
     * @param [OUT] txPower The tx power index which was set.
     * @param [OUT] txTimeOnAir The time-on-air of the frame.
     * @retval Returns true, if the configuration was applied successfully.
     */
    bool RegionTxConfig(uint8_t region, RPTxConfigParams* txConfig, int8_t* txPower, uint32_t* txTimeOnAir);

    /**
     * @brief The function processes a Link ADR Request.
     * @param [IN] region LoRaWAN region.
     * @param [IN] linkAdrReq Pointer to the function parameters.
     * @param [OUT] drOut The datarate which was applied.
     * @param [OUT] txPowOut The TX power which was applied.
     * @param [OUT] nbRepOut The number of repetitions to apply.
     * @param [OUT] nbBytesParsed The number bytes which were parsed.
     * @retval Returns the status of the operation, according to the LoRaMAC specification.
     */
    uint8_t RegionLinkAdrReq(uint8_t region, RPLinkAdrReqParams* linkAdrReq, int8_t* drOut, int8_t* txPowOut, uint8_t* nbRepOut, uint8_t* nbBytesParsed);

    /**
     * @brief The function processes a RX Parameter Setup Request.
     *
     * @param [IN] region LoRaWAN region.
     *
     * @param [IN] rxParamSetupReq Pointer to the function parameters.
     *
     * @retval Returns the status of the operation, according to the LoRaMAC specification.
     */
    uint8_t RegionRxParamSetupReq(uint8_t region, RPRxParamSetupReqParams* rxParamSetupReq);

    /**
     * @brief The function processes a New Channel Request.
     * @param [IN] region LoRaWAN region.
     * @param [IN] newChannelReq Pointer to the function parameters.
     * @retval Returns the status of the operation, according to the LoRaMAC specification.
     */
    int8_t RegionNewChannelReq(uint8_t region, RPNewChannelReqParams* newChannelReq);

    /**
     * @brief The function processes a TX ParamSetup Request.
     * @param [IN] region LoRaWAN region.
     * @param [IN] txParamSetupReq Pointer to the function parameters.
     * @retval Returns the status of the operation, according to the LoRaMAC specification.
     *         Returns -1, if the functionality is not implemented. In this case, the end node
     *         shall ignore the command.
     */
    int8_t RegionTxParamSetupReq(uint8_t region, RPTxParamSetupReqParams* txParamSetupReq);

    /**
     * @brief The function processes a DlChannel Request.
     * @param [IN] region LoRaWAN region.
     * @param [IN] dlChannelReq Pointer to the function parameters.
     * @retval Returns the status of the operation, according to the LoRaMAC specification.
     */
    int8_t RegionDlChannelReq(uint8_t region, RPDlChannelReqParams* dlChannelReq);

    /**
     * @brief Alternates the datarate of the channel for the join request.
     * @param [IN] region LoRaWAN region.
     * @param [IN] currentDr Current datarate.
     * @param [IN] type Alternation type.
     * @retval Datarate to apply.
     */
    int8_t RegionAlternateDr(uint8_t region, int8_t currentDr, RPAlternateDrType type);

    /**
     * @brief Searches and set the next random available channel
     * @param [IN] region LoRaWAN region.
     * @param [OUT] channel Next channel to use for TX.
     * @param [OUT] time Time to wait for the next transmission according to the duty cycle.
     * @param [OUT] aggregatedTimeOff Updates the aggregated time off.
     * @retval Function status [true: OK, false: Unable to find a channel on the current datarate].
     */
    bool RegionNextChannel(uint8_t region, RPNextChanParams* nextChanParams, uint8_t* channel, uint32_t* time, uint32_t* aggregatedTimeOff);

    /**
     * @brief Adds a channel.
     * @param [IN] region LoRaWAN region.
     * @param [IN] channelAdd Pointer to the function parameters.
     * @retval Status of the operation.
     */
    bool RegionChannelAdd(uint8_t region, RPChannelAddParams* channelAdd);

    /**
     * @brief Removes a channel.
     * @param [IN] region LoRaWAN region.
     * @param [IN] channelRemove Pointer to the function parameters.
     * @retval Returns true, if the channel was removed successfully.
     */
    bool RegionChannelsRemove(uint8_t region, RPChannelRemoveParams* channelRemove);

    /**
     * @brief Computes new datarate according to the given offset
     * @param [IN] downlinkDwellTime Downlink dwell time configuration. 0: No limit, 1: 400ms
     * @param [IN] dr Current datarate
     * @param [IN] drOffset Offset to be applied
     * @retval newDr Computed datarate.
     */
    uint8_t RegionApplyDrOffset(uint8_t region, uint8_t downlinkDwellTime, int8_t dr, int8_t drOffset);

    /**
     * @brief Sets the radio into beacon reception mode
     * @param [IN] rxBeaconSetup Pointer to the function parameters
     * @param [out] outDr Datarate used to receive the beacon
     */
    void RegionRxBeaconSetup(uint8_t region, RPRxBeaconSetup* rxBeaconSetup, uint8_t* outDr);
};
