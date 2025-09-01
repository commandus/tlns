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

#include "lorawan/regional-parameters/regional-parameters.h"

#include <cmath>
#include <cstring>

/**
 * Compute bit of a channel index.
 */
#define LC(channelIndex) ( uint16_t )( 1 << ( channelIndex - 1 ) )

bool RegionCommonChanDisable(
    std::vector <uint16_t> &channelsMask,
    uint8_t id,
    uint8_t maxChannels
)
{
    uint8_t index = id / 16;
    if ((index > (maxChannels / 16)) || (id >= maxChannels))
        return false;
    // Deactivate channel
    channelsMask[index] &= ~(1 << (id % 16));
    return true;
}

void RegionCommonRxBeaconSetup(
    RPRegionCommonRxBeaconSetupParams* rxBeaconSetupParams
)
{
    bool rxContinuous = true;
    uint8_t datarate;
    // Check the RX continuous mode
    if (rxBeaconSetupParams->RxTime != 0)
        rxContinuous = false;
    // Get region specific datarate
    datarate = rxBeaconSetupParams->Datarates[rxBeaconSetupParams->BeaconDatarate];
    // Setup radio
}

RegionalParameters::RegionalParameters()
{

}

RegionalParameters::RegionalParameters(
    const RegionalParameters &value
)
    : RegionBands(value.RegionBands)
{
}

RegionalParameters::RegionalParameters(uint8_t id) {
}

std::string RegionalParameters::toString() const {
    return "";
}

void RegionalParameters::setValue(
    int frequency,
    int minDR,
    int maxDR,
    bool enabled,
    bool custom
) {
}
    
/**
* @brief Verifies if a region is active or not. If a region is not active, it cannot be used.
* @retval Return true, if the region is supported.
*/
bool RegionalParameters::RegionIsActive() {
    return true;
}

/**
 * @brief The function gets a value of a specific phy attribute.
 * @param getPhy Pointer to the function parameters.
 * @retval Returns a structure containing the PHY parameter.
 */
RPPhyParam RegionalParameters::RegionGetPhyParam(
    RPGetPhyParams* getPhy
) {
    const RPPhyParam p {0};
    return p;
}

#define BACKOFF_DC_1_HOUR                   100

uint16_t GetDutyCycle(
    RPBand* band,
    bool joined,
    RPSysTime elapsedTimeSinceStartup
)
{
    uint16_t dutyCycle = band->DCycle;
    if (!joined) {
        uint16_t joinDutyCycle = BACKOFF_DC_1_HOUR;
        // Take the most restrictive duty cycle
        dutyCycle = std::max(dutyCycle, joinDutyCycle);
    }

    // Prevent value of 0
    if (dutyCycle == 0)
        dutyCycle = 1;
    return dutyCycle;
}

/**
 * @brief Sets the last tx done property. This is a generic function and valid for all regions.
 * @param [IN,OUT] band The band to be updated.
 * @param [IN] lastTxAirTime The time on air of the last TX frame.
 * @param [IN] joined Set to true if the device has joined.
 * @param [IN] elapsedTimeSinceStartup Elapsed time since initialization.
 */
void RegionCommonSetBandTxDone(
    RPBand* band,
    uint32_t lastTxAirTime,
    bool joined,
    RPSysTime elapsedTimeSinceStartup
) {
    // Get the band duty cycle. If not joined, the function either returns the join duty cycle
    // or the band duty cycle, whichever is more restrictive.
    uint16_t dutyCycle = GetDutyCycle(band, joined, elapsedTimeSinceStartup );
    // Reduce with transmission time
    if (band->TimeCredits > ( lastTxAirTime * dutyCycle)) {
        // Reduce time credits by the time of air
        band->TimeCredits -= ( lastTxAirTime * dutyCycle );
    } else
        band->TimeCredits = 0;
}

/**
 * @brief Updates the last TX done parameters of the current channel.
 * @param [IN] txDone Pointer to the function parameters.
 */
void RegionalParameters::RegionSetBandTxDone(
    RPSetBandTxDoneParams* txDone
) {
    (&RegionBands[RegionNvmGroup2->Channels[txDone->Channel].Band],
        txDone->LastTxAirTime, txDone->Joined, txDone->ElapsedTimeSinceStartUp );
}

static void RegionCommonChanMaskCopy(
    std::vector<uint16_t> &channelsMaskDest,
    std::vector<uint16_t> &channelsMaskSrc
)
{
    auto minSz = std::min(channelsMaskDest.size(), channelsMaskSrc.size());
    for (auto i = 0; i < minSz; i++)
        channelsMaskDest[i] = channelsMaskSrc[i];
}


int8_t RegionCommonComputeTxPower(
    int8_t txPowerIndex,
    float maxEirp,
    float antennaGain
)
{
    return (int8_t) std::floor((maxEirp - (txPowerIndex * 2U)) - antennaGain);
}

/**
 * @brief Initializes the channels masks and the channels.
 * @param [IN] params Pointer to the function parameters.
 */
void RegionalParameters::RegionInitDefaults(
    RPInitDefaultsParams* params
) {
    switch(params->Type) {
        case INIT_TYPE_DEFAULTS:
        {
            if (!params->NvmGroup1 || !params->NvmGroup2)
                return;

            // Default channels

            // Apply frequency offset

            // Default ChannelsMask
            RegionNvmGroup2->ChannelsDefaultMask[0] = LC(1) + LC(2);

            // Update the channels mask
            RegionCommonChanMaskCopy( RegionNvmGroup2->ChannelsMask, RegionNvmGroup2->ChannelsDefaultMask);
            break;
        }
        case INIT_TYPE_RESET_TO_DEFAULT_CHANNELS:
            // Reset Channels Rx1Frequency to default 0
            RegionNvmGroup2->Channels[0].Rx1Frequency = 0;
            RegionNvmGroup2->Channels[1].Rx1Frequency = 0;
            // Update the channels mask
            RegionCommonChanMaskCopy( RegionNvmGroup2->ChannelsMask, RegionNvmGroup2->ChannelsDefaultMask);
            break;
        case INIT_TYPE_ACTIVATE_DEFAULT_CHANNELS:
            // Activate channels default mask
            RegionNvmGroup2->ChannelsMask[0] |= RegionNvmGroup2->ChannelsDefaultMask[0];
            break;
        default:
            break;
    }

}

bool RegionCommonValueInRange(
    int8_t value,
    int8_t min,
    int8_t max
)
{
    return ((value >= min) && (value <= max));
}

/**
 * @brief Verifies a parameter.
 * @param [IN] verify Pointer to the function parameters.
 * @param [IN] type Sets the initialization type.
 * @retval Returns true, if the parameter is valid.
 */
bool RegionalParameters::RegionVerify(
    RPVerifyParams* verify,
    RPPhyAttribute phyAttribute
) {
    return true;
}

/**
 * @brief The function parses the input buffer and sets up the channels of the
 *        CF list.
 * @param [IN] applyCFList Pointer to the function parameters.
 */
void RegionalParameters::RegionApplyCFList(
    RPApplyCFListParams* applyCFList
) {
}

/**
 * @brief Sets a channels mask.
 * @param [IN] chanMaskSet Pointer to the function parameters.
 * @retval Returns true, if the channels mask could be set.
 */
bool RegionChanMaskSet(
    RPChanMaskSetParams* chanMaskSet
) {
    return false;
}

/**
 * @brief Configuration of the RX windows.
 * @param [IN] rxConfig Pointer to the function parameters.
 * @param [OUT] datarate The datarate index which was set.
 * @retval Returns true, if the configuration was applied successfully.
 */
bool RegionalParameters::RegionRxConfig(
    RPRxConfigParams* rxConfig,
    int8_t* datarate
) {
    return false;
}

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
void RegionalParameters::RegionComputeRxWindowParameters(
    int8_t datarate,
    uint8_t minRxSymbols,
    uint32_t rxError,
    RPRxConfigParams *rxConfigParams
) {

}

/**
 * @brief TX configuration.
 * @param [IN] txConfig Pointer to the function parameters.
 * @param [OUT] txPower The tx power index which was set.
 * @param [OUT] txTimeOnAir The time-on-air of the frame.
 * @retval Returns true, if the configuration was applied successfully.
 */
bool RegionalParameters::RegionTxConfig(
    RPTxConfigParams* txConfig,
    int8_t* txPower,
    uint32_t* txTimeOnAir
) {

}

/**
 * @brief The function processes a Link ADR Request.
 * @param [IN] linkAdrReq Pointer to the function parameters.
 * @param [OUT] drOut The datarate which was applied.
 * @param [OUT] txPowOut The TX power which was applied.
 * @param [OUT] nbRepOut The number of repetitions to apply.
 * @param [OUT] nbBytesParsed The number bytes which were parsed.
 * @retval Returns the status of the operation, according to the LoRaMAC specification.
 */
uint8_t RegionalParameters::RegionLinkAdrReq(
    RPLinkAdrReqParams* linkAdrReq,
    int8_t* drOut,
    int8_t* txPowOut,
    uint8_t* nbRepOut,
    uint8_t* nbBytesParsed
) {
    return 0;
}

/**
 * @brief The function processes a RX Parameter Setup Request.
 * @param [IN] rxParamSetupReq Pointer to the function parameters.
 * @retval Returns the status of the operation, according to the LoRaMAC specification.
 */
uint8_t RegionalParameters::RegionRxParamSetupReq(
    RPRxParamSetupReqParams* rxParamSetupReq
) {
    return 0;
}

/**
 * @brief The function processes a New Channel Request.
 * @param [IN] newChannelReq Pointer to the function parameters.
 * @retval Returns the status of the operation, according to the LoRaMAC specification.
 */
int8_t RegionalParameters::RegionNewChannelReq(
    RPNewChannelReqParams* newChannelReq
) {
    return 0;
}

/**
 * @brief The function processes a TX ParamSetup Request.
 * @param [IN] txParamSetupReq Pointer to the function parameters.
 * @retval Returns the status of the operation, according to the LoRaMAC specification.
 *         Returns -1, if the functionality is not implemented. In this case, the end node
 *         shall ignore the command.
 */
int8_t RegionalParameters::RegionTxParamSetupReq(
    RPTxParamSetupReqParams* txParamSetupReq
) {
}

/**
 * @brief The function processes a DlChannel Request.
 * @param [IN] dlChannelReq Pointer to the function parameters.
 * @retval Returns the status of the operation, according to the LoRaMAC specification.
 */
int8_t RegionalParameters::RegionDlChannelReq(
    RPDlChannelReqParams* dlChannelReq
) {
    return 0;
}

/**
 * @brief Alternates the datarate of the channel for the join request.
 * @param [IN] region LoRaWAN region.
 * @param [IN] currentDr Current datarate.
 * @param [IN] type Alternation type.
 * @retval Datarate to apply.
 */
int8_t RegionalParameters::RegionAlternateDr(
    int8_t currentDr,
    RPAlternateDrType type
) {
    return 0;
}

/**
 * @brief Searches and set the next random available channel
 * @param [OUT] channel Next channel to use for TX.
 * @param [OUT] time Time to wait for the next transmission according to the duty cycle.
 * @param [OUT] aggregatedTimeOff Updates the aggregated time off.
 * @retval Function status [true: OK, false: Unable to find a channel on the current datarate].
 */
bool RegionalParameters::RegionNextChannel(
    RPNextChanParams* nextChanParams,
    uint8_t* channel,
    uint32_t* time,
    uint32_t* aggregatedTimeOff
) {
    return false;
}

/**
 * @brief Adds a channel.
 * @param [IN] channelAdd Pointer to the function parameters.
 * @retval Status of the operation.
 */
bool RegionalParameters::RegionChannelAdd(
    RPChannelAddParams* channelAdd
) {
    return false;
}

/**
 * @brief Removes a channel.
 * @param [IN] channelRemove Pointer to the function parameters.
 * @retval Returns true, if the channel was removed successfully.
 */
bool RegionalParameters::RegionChannelsRemove(
    RPChannelRemoveParams* channelRemove
) {
    return false;
}

/**
 * @brief Computes new datarate according to the given offset
 * @param [IN] downlinkDwellTime Downlink dwell time configuration. 0: No limit, 1: 400ms
 * @param [IN] dr Current datarate
 * @param [IN] drOffset Offset to be applied
 * @retval newDr Computed datarate.
 */
uint8_t RegionalParameters::RegionApplyDrOffset(
    uint8_t downlinkDwellTime,
    int8_t dr,
    int8_t drOffset
) {
    return 0;
}

/**
 * @brief Sets the radio into beacon reception mode
 * @param [IN] rxBeaconSetup Pointer to the function parameters
 * @param [out] outDr Datarate used to receive the beacon
 */
void RegionalParameters::RegionRxBeaconSetup(
    RPRxBeaconSetup* rxBeaconSetup,
    uint8_t* outDr) {
}

//------------------------------------------------------------------------------------------------------

#define REGION_PTR(a, region) a[region]
#define REGION_EXISTS(a, region) (a[region] != nullptr)
#define REGION_ACTIVE(a, region) (REGION_EXISTS(a, region) ? a[region]->RegionIsActive() : false)

RegionalParametersCollection::RegionalParametersCollection()
    : value{}
{
}

RegionalParametersCollection::RegionalParametersCollection(
    const RegionalParametersCollection &coll
) {
    memmove(value, coll.value, sizeof(value));
}

std::string RegionalParametersCollection::toString() const {
    return "";
}

/**
* @brief Verifies if a region is active or not. If a region is not active, it cannot be used.
* @param region LoRaWAN region.
* @retval Return true, if the region is supported.
*/
bool RegionalParametersCollection::RegionIsActive(uint8_t region) {
    return REGION_ACTIVE(value, region);
}

/**
 * @brief The function gets a value of a specific phy attribute.
 * @param region LoRaWAN region.
 * @param getPhy Pointer to the function parameters.
 * @retval Returns a structure containing the PHY parameter.
 */
RPPhyParam RegionalParametersCollection::RegionGetPhyParam(
    uint8_t region,
    RPGetPhyParams* getPhy
) {
    if (REGION_EXISTS(value, region))
        return REGION_PTR(value, region)->RegionGetPhyParam(getPhy);
    return { 0 };
}

/**
 * @brief Updates the last TX done parameters of the current channel.
 * @param [IN] region LoRaWAN region.
 * @param [IN] txDone Pointer to the function parameters.
 */
void RegionalParametersCollection::RegionSetBandTxDone(
    uint8_t region,
    RPSetBandTxDoneParams* txDone
) {
    if (REGION_EXISTS(value, region))
        REGION_PTR(value, region)->RegionSetBandTxDone(txDone);
}

/**
 * @brief Initializes the channels masks and the channels.
 * @param [IN] region LoRaWAN region.
 * @param [IN] params Pointer to the function parameters.
 */
void RegionalParametersCollection::RegionInitDefaults(
    uint8_t region,
    RPInitDefaultsParams* params
) {
    if (REGION_EXISTS(value, region))
        REGION_PTR(value, region)->RegionInitDefaults(params);
}

/**
 * @brief Verifies a parameter.
 * @param [IN] region LoRaWAN region.
 * @param [IN] verify Pointer to the function parameters.
 * @param [IN] type Sets the initialization type.
 * @retval Returns true, if the parameter is valid.
 */
bool RegionalParametersCollection::RegionVerify(
    uint8_t region,
    RPVerifyParams* verify,
    RPPhyAttribute phyAttribute
) {
    if (REGION_EXISTS(value, region))
        REGION_PTR(value, region)->RegionVerify(verify, phyAttribute);
    return 0;
}

/**
 * @brief The function parses the input buffer and sets up the channels of the
 *        CF list.
 *
 * @param [IN] region LoRaWAN region.
 *
 * @param [IN] applyCFList Pointer to the function parameters.
 */
void RegionalParametersCollection::RegionApplyCFList(
    uint8_t region,
    RPApplyCFListParams* applyCFList
) {
    if (REGION_EXISTS(value, region))
        REGION_PTR(value, region)->RegionApplyCFList(applyCFList);
}

/**
 * @brief Sets a channels mask.
 * @param [IN] region LoRaWAN region.
 * @param [IN] chanMaskSet Pointer to the function parameters.
 * @retval Returns true, if the channels mask could be set.
 */
bool RegionalParametersCollection::RegionChanMaskSet(
    uint8_t region,
    RPChanMaskSetParams* chanMaskSet
) {
    if (REGION_EXISTS(value, region))
        return REGION_PTR(value, region)->RegionChanMaskSet(chanMaskSet);
    return false;
}

/**
 * @brief Configuration of the RX windows.
 * @param [IN] region LoRaWAN region.
 * @param [IN] rxConfig Pointer to the function parameters.
 * @param [OUT] datarate The datarate index which was set.
 * @retval Returns true, if the configuration was applied successfully.
 */
bool RegionalParametersCollection::RegionRxConfig(
    uint8_t region,
    RPRxConfigParams* rxConfig,
    int8_t* datarate
) {
    if (REGION_EXISTS(value, region))
        return REGION_PTR(value, region)->RegionRxConfig(rxConfig, datarate);
    return false;
}

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
void RegionalParametersCollection::RegionComputeRxWindowParameters(
    uint8_t region,
    int8_t datarate,
    uint8_t minRxSymbols,
    uint32_t rxError,
    RPRxConfigParams *rxConfigParams
) {
    if (REGION_EXISTS(value, region))
        REGION_PTR(value, region)->RegionComputeRxWindowParameters(datarate, minRxSymbols, rxError, rxConfigParams);
}

/**
 * @brief TX configuration.
 * @param [IN] region LoRaWAN region.
 * @param [IN] txConfig Pointer to the function parameters.
 * @param [OUT] txPower The tx power index which was set.
 * @param [OUT] txTimeOnAir The time-on-air of the frame.
 * @retval Returns true, if the configuration was applied successfully.
 */
bool RegionalParametersCollection::RegionTxConfig(
    uint8_t region,
    RPTxConfigParams* txConfig,
    int8_t* txPower,
    uint32_t* txTimeOnAir
) {
    if (REGION_EXISTS(value, region))
        return REGION_PTR(value, region)->RegionTxConfig(txConfig, txPower, txTimeOnAir);
    return false;
}

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
uint8_t RegionalParametersCollection::RegionLinkAdrReq(
    uint8_t region,
    RPLinkAdrReqParams* linkAdrReq,
    int8_t* drOut,
    int8_t* txPowOut,
    uint8_t* nbRepOut,
    uint8_t* nbBytesParsed
) {
    if (REGION_EXISTS(value, region))
        return REGION_PTR(value, region)->RegionLinkAdrReq(linkAdrReq, drOut, txPowOut, nbRepOut, nbBytesParsed);
    return 0;
}

/**
 * @brief The function processes a RX Parameter Setup Request.
 * @param [IN] region LoRaWAN region.
 * @param [IN] rxParamSetupReq Pointer to the function parameters.
 * @retval Returns the status of the operation, according to the LoRaMAC specification.
 */
uint8_t RegionalParametersCollection::RegionRxParamSetupReq(
    uint8_t region,
    RPRxParamSetupReqParams* rxParamSetupReq
) {
    if (REGION_EXISTS(value, region))
        return REGION_PTR(value, region)->RegionRxParamSetupReq(rxParamSetupReq);
    return 0;
}

/**
 * @brief The function processes a New Channel Request.
 * @param [IN] region LoRaWAN region.
 * @param [IN] newChannelReq Pointer to the function parameters.
 * @retval Returns the status of the operation, according to the LoRaMAC specification.
 */
int8_t RegionalParametersCollection::RegionNewChannelReq(
    uint8_t region,
    RPNewChannelReqParams* newChannelReq
) {
    if (REGION_EXISTS(value, region))
        return REGION_PTR(value, region)->RegionNewChannelReq(newChannelReq);
    return 0;
}

/**
 * @brief The function processes a TX ParamSetup Request.
 * @param [IN] region LoRaWAN region.
 * @param [IN] txParamSetupReq Pointer to the function parameters.
 * @retval Returns the status of the operation, according to the LoRaMAC specification.
 *         Returns -1, if the functionality is not implemented. In this case, the end node
 *         shall ignore the command.
 */
int8_t RegionalParametersCollection::RegionTxParamSetupReq(
    uint8_t region,
    RPTxParamSetupReqParams* txParamSetupReq
) {
    if (REGION_EXISTS(value, region))
        return REGION_PTR(value, region)->RegionTxParamSetupReq(txParamSetupReq);
    return 0;
}

/**
 * @brief The function processes a DlChannel Request.
 * @param [IN] region LoRaWAN region.
 * @param [IN] dlChannelReq Pointer to the function parameters.
 * @retval Returns the status of the operation, according to the LoRaMAC specification.
 */
int8_t RegionalParametersCollection::RegionDlChannelReq(
    uint8_t region,
    RPDlChannelReqParams* dlChannelReq
) {
    if (REGION_EXISTS(value, region))
        return REGION_PTR(value, region)->RegionDlChannelReq(dlChannelReq);
    return 0;
}

/**
 * @brief Alternates the datarate of the channel for the join request.
 * @param [IN] region LoRaWAN region.
 * @param [IN] currentDr Current datarate.
 * @param [IN] type Alternation type.
 * @retval Datarate to apply.
 */
int8_t RegionalParametersCollection::RegionAlternateDr(
    uint8_t region,
    int8_t currentDr,
    RPAlternateDrType type
) {
    if (REGION_EXISTS(value, region))
        return REGION_PTR(value, region)->RegionAlternateDr(currentDr, type);
    return 0;
}

/**
 * @brief Searches and set the next random available channel
 * @param [IN] region LoRaWAN region.
 * @param [OUT] channel Next channel to use for TX.
 * @param [OUT] time Time to wait for the next transmission according to the duty cycle.
 * @param [OUT] aggregatedTimeOff Updates the aggregated time off.
 * @retval Function status [true: OK, false: Unable to find a channel on the current datarate].
 */
bool RegionalParametersCollection::RegionNextChannel(
    uint8_t region,
    RPNextChanParams* nextChanParams,
    uint8_t* channel,
    uint32_t* time,
    uint32_t* aggregatedTimeOff
) {
    if (REGION_EXISTS(value, region))
        return REGION_PTR(value, region)->RegionNextChannel(nextChanParams, channel, time, aggregatedTimeOff);
    return false;
}

/**
 * @brief Adds a channel.
 * @param [IN] region LoRaWAN region.
 * @param [IN] channelAdd Pointer to the function parameters.
 * @retval Status of the operation.
 */
bool RegionalParametersCollection::RegionChannelAdd(
    uint8_t region,
    RPChannelAddParams* channelAdd
) {
    if (REGION_EXISTS(value, region))
        return REGION_PTR(value, region)->RegionChannelAdd(channelAdd);
    return false;
}

/**
 * @brief Removes a channel.
 * @param [IN] region LoRaWAN region.
 * @param [IN] channelRemove Pointer to the function parameters.
 * @retval Returns true, if the channel was removed successfully.
 */
bool RegionalParametersCollection::RegionChannelsRemove(
    uint8_t region,
    RPChannelRemoveParams* channelRemove
) {
    if (REGION_EXISTS(value, region))
        return REGION_PTR(value, region)->RegionChannelsRemove(channelRemove);
    return false;
}

/**
 * @brief Computes new datarate according to the given offset
 * @param [IN] downlinkDwellTime Downlink dwell time configuration. 0: No limit, 1: 400ms
 * @param [IN] dr Current datarate
 * @param [IN] drOffset Offset to be applied
 * @retval newDr Computed datarate.
 */
uint8_t RegionalParametersCollection::RegionApplyDrOffset(
    uint8_t region,
    uint8_t downlinkDwellTime,
    int8_t dr,
    int8_t drOffset
) {
    if (REGION_EXISTS(value, region))
        return REGION_PTR(value, region)->RegionApplyDrOffset(downlinkDwellTime, dr, drOffset);
    return false;
}

/**
 * @brief Sets the radio into beacon reception mode
 * @param [IN] rxBeaconSetup Pointer to the function parameters
 * @param [out] outDr Datarate used to receive the beacon
 */
void RegionalParametersCollection::RegionRxBeaconSetup(
    uint8_t region,
    RPRxBeaconSetup* rxBeaconSetup,
    uint8_t* outDr
) {
    if (REGION_EXISTS(value, region))
        REGION_PTR(value, region)->RegionRxBeaconSetup(rxBeaconSetup, outDr);
}
