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

#include "lorawan/regional-parameters/regional-parameters-AS923.h"

/// LoRaMac maximum number of channels
#define AS923_MAX_NB_CHANNELS                       16

/// Number of default channels
#define AS923_NUMB_DEFAULT_CHANNELS                 2

/// Number of channels to apply for the CF list
#define AS923_NUMB_CHANNELS_CF_LIST                 5

RegionalParametersAS923::RegionalParametersAS923()
{

}

RegionalParametersAS923::RegionalParametersAS923(
    const RegionalParametersAS923 &value
)
{
}

/**
 * @brief The function gets a value of a specific phy attribute.
 * @param getPhy Pointer to the function parameters.
 * @retval Returns a structure containing the PHY parameter.
 */
RPPhyParam RegionalParametersAS923::RegionGetPhyParam(
    RPGetPhyParams* getPhy
) {
    const RPPhyParam p {0};
    return p;
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
 * @brief Initializes the channels masks and the channels.
 * @param [IN] params Pointer to the function parameters.
 */
void RegionalParametersAS923::RegionInitDefaults(
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
            // RegionCommonChanMaskCopy( RegionNvmGroup2->ChannelsMask, RegionNvmGroup2->ChannelsDefaultMask, 0);

            break;
        }
        case INIT_TYPE_RESET_TO_DEFAULT_CHANNELS:
            // Reset Channels Rx1Frequency to default 0
            RegionNvmGroup2->Channels[0].Rx1Frequency = 0;
            RegionNvmGroup2->Channels[1].Rx1Frequency = 0;
            // Update the channels mask
            RegionCommonChanMaskCopy( RegionNvmGroup2->ChannelsMask, RegionNvmGroup2->ChannelsDefaultMask, CHANNELS_MASK_SIZE );
            break;
        case INIT_TYPE_ACTIVATE_DEFAULT_CHANNELS:
            // Activate channels default mask
            RegionNvmGroup2->ChannelsMask[0] |= RegionNvmGroup2->ChannelsDefaultMask[0];
            break;
        default:
            break;
    }

}

/**
 * @brief Verifies a parameter.
 * @param [IN] verify Pointer to the function parameters.
 * @param [IN] type Sets the initialization type.
 * @retval Returns true, if the parameter is valid.
 */
bool RegionalParametersAS923::RegionVerify(
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
void RegionalParametersAS923::RegionApplyCFList(
    RPApplyCFListParams* applyCFList
) {
    RPChannelParams newChannel;
    RPChannelAddParams channelAdd;
    RPChannelRemoveParams channelRemove;

    // Setup default datarate range
    newChannel.DrRange.Value = (DR_5 << 4) | DR_0;

    // Size of the optional CF list
    if (applyCFList->Size != 16)
        return;

    // Last byte CFListType must be 0 to indicate the CFList contains a list of frequencies
    if (applyCFList->Payload[15] != 0)
        return;

    // Last byte is RFU, don't take it into account
    for (uint8_t i = 0, chanIdx = AS923_NUMB_DEFAULT_CHANNELS; chanIdx < AS923_MAX_NB_CHANNELS; i+=3, chanIdx++) {
        if (chanIdx < (AS923_NUMB_CHANNELS_CF_LIST + AS923_NUMB_DEFAULT_CHANNELS)) {
            // Channel frequency
            newChannel.Frequency = (uint32_t) applyCFList->Payload[i];
            newChannel.Frequency |= ( (uint32_t) applyCFList->Payload[i + 1] << 8 );
            newChannel.Frequency |= ( (uint32_t) applyCFList->Payload[i + 2] << 16 );
            newChannel.Frequency *= 100;
            // Initialize alternative frequency to 0
            newChannel.Rx1Frequency = 0;
        } else {
            newChannel.Frequency = 0;
            newChannel.DrRange.Value = 0;
            newChannel.Rx1Frequency = 0;
        }

        if (newChannel.Frequency != 0) {
            channelAdd.NewChannel = &newChannel;
            channelAdd.ChannelId = chanIdx;
            // Try to add all channels
            RegionChannelAdd(&channelAdd);
        } else {
            channelRemove.ChannelId = chanIdx;
            RegionChannelsRemove(&channelRemove);
        }
    }
}

/**
 * @brief Sets a channels mask.
 * @param [IN] chanMaskSet Pointer to the function parameters.
 * @retval Returns true, if the channels mask could be set.
 */
bool RegionChanMaskSet(
    RPChanMaskSetParams* chanMaskSet
) {

}

/**
 * @brief Configuration of the RX windows.
 * @param [IN] rxConfig Pointer to the function parameters.
 * @param [OUT] datarate The datarate index which was set.
 * @retval Returns true, if the configuration was applied successfully.
 */
bool RegionalParametersAS923::RegionRxConfig(
    RPRxConfigParams* rxConfig,
    int8_t* datarate
) {
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
void RegionalParametersAS923::RegionComputeRxWindowParameters(
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
bool RegionalParametersAS923::RegionTxConfig(
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
uint8_t RegionalParametersAS923::RegionLinkAdrReq(
    RPLinkAdrReqParams* linkAdrReq,
    int8_t* drOut,
    int8_t* txPowOut,
    uint8_t* nbRepOut,
    uint8_t* nbBytesParsed
) {
}

/**
 * @brief The function processes a RX Parameter Setup Request.
 * @param [IN] rxParamSetupReq Pointer to the function parameters.
 * @retval Returns the status of the operation, according to the LoRaMAC specification.
 */
uint8_t RegionalParametersAS923::RegionRxParamSetupReq(
    RPRxParamSetupReqParams* rxParamSetupReq
) {
}

/**
 * @brief The function processes a New Channel Request.
 * @param [IN] newChannelReq Pointer to the function parameters.
 * @retval Returns the status of the operation, according to the LoRaMAC specification.
 */
int8_t RegionalParametersAS923::RegionNewChannelReq(
    RPNewChannelReqParams* newChannelReq
) {

}

/**
 * @brief The function processes a TX ParamSetup Request.
 * @param [IN] txParamSetupReq Pointer to the function parameters.
 * @retval Returns the status of the operation, according to the LoRaMAC specification.
 *         Returns -1, if the functionality is not implemented. In this case, the end node
 *         shall ignore the command.
 */
int8_t RegionalParametersAS923::RegionTxParamSetupReq(
    RPTxParamSetupReqParams* txParamSetupReq
) {
}

/**
 * @brief The function processes a DlChannel Request.
 * @param [IN] dlChannelReq Pointer to the function parameters.
 * @retval Returns the status of the operation, according to the LoRaMAC specification.
 */
int8_t RegionalParametersAS923::RegionDlChannelReq(
    RPDlChannelReqParams* dlChannelReq
) {
}

/**
 * @brief Alternates the datarate of the channel for the join request.
 * @param [IN] region LoRaWAN region.
 * @param [IN] currentDr Current datarate.
 * @param [IN] type Alternation type.
 * @retval Datarate to apply.
 */
int8_t RegionalParametersAS923::RegionAlternateDr(
    int8_t currentDr,
    RPAlternateDrType type
) {
}

/**
 * @brief Searches and set the next random available channel
 * @param [OUT] channel Next channel to use for TX.
 * @param [OUT] time Time to wait for the next transmission according to the duty cycle.
 * @param [OUT] aggregatedTimeOff Updates the aggregated time off.
 * @retval Function status [true: OK, false: Unable to find a channel on the current datarate].
 */
bool RegionalParametersAS923::RegionNextChannel(
    RPNextChanParams* nextChanParams,
    uint8_t* channel,
    uint32_t* time,
    uint32_t* aggregatedTimeOff
) {
}

/**
 * @brief Adds a channel.
 * @param [IN] channelAdd Pointer to the function parameters.
 * @retval Status of the operation.
 */
bool RegionalParametersAS923::RegionChannelAdd(
    RPChannelAddParams* channelAdd
) {
}

/**
 * @brief Removes a channel.
 * @param [IN] channelRemove Pointer to the function parameters.
 * @retval Returns true, if the channel was removed successfully.
 */
bool RegionalParametersAS923::RegionChannelsRemove(
    RPChannelRemoveParams* channelRemove
) {
    uint8_t id = channelRemove->ChannelId;
    if (id < AS923_NUMB_DEFAULT_CHANNELS)
        return false;

    // Remove the channel from the list of channels
    RegionNvmGroup2->Channels[id] = (RPChannelParams) { 0, 0, { 0 }, 0 };
    return RegionCommonChanDisable(RegionNvmGroup2->ChannelsMask, id, AS923_MAX_NB_CHANNELS);

}

/**
 * @brief Computes new datarate according to the given offset
 * @param [IN] downlinkDwellTime Downlink dwell time configuration. 0: No limit, 1: 400ms
 * @param [IN] dr Current datarate
 * @param [IN] drOffset Offset to be applied
 * @retval newDr Computed datarate.
 */
uint8_t RegionalParametersAS923::RegionApplyDrOffset(
    uint8_t downlinkDwellTime,
    int8_t dr,
    int8_t drOffset
) {
    RPRegionCommonRxBeaconSetupParams regionCommonRxBeaconSetup;

    regionCommonRxBeaconSetup.Datarates = DataratesAS923;
    regionCommonRxBeaconSetup.Frequency = rxBeaconSetup->Frequency;
    regionCommonRxBeaconSetup.BeaconSize = AS923_BEACON_SIZE;
    regionCommonRxBeaconSetup.BeaconDatarate = AS923_BEACON_CHANNEL_DR;
    regionCommonRxBeaconSetup.BeaconChannelBW = AS923_BEACON_CHANNEL_BW;
    regionCommonRxBeaconSetup.RxTime = rxBeaconSetup->RxTime;
    regionCommonRxBeaconSetup.SymbolTimeout = rxBeaconSetup->SymbolTimeout;

    RegionCommonRxBeaconSetup(&regionCommonRxBeaconSetup);

    // Store downlink datarate
    *outDr = AS923_BEACON_CHANNEL_DR;
}

/**
 * @brief Sets the radio into beacon reception mode
 * @param [IN] rxBeaconSetup Pointer to the function parameters
 * @param [out] outDr Datarate used to receive the beacon
 */
void RegionalParametersAS923::RegionRxBeaconSetup(
    RPRxBeaconSetup* rxBeaconSetup,
    uint8_t* outDr)
{
}
