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

class RegionalParametersAS923: public RegionalParameters {
public:
    RegionalParametersAS923();
    RegionalParametersAS923(const RegionalParametersAS923 &value);
    /**
     * @brief The function gets a value of a specific phy attribute.
     * @param getPhy Pointer to the function parameters.
     * @retval Returns a structure containing the PHY parameter.
     */
    RPPhyParam RegionGetPhyParam(RPGetPhyParams* getPhy) override;

    // default methods (not overriden)
    // void RegionSetBandTxDone(RPSetBandTxDoneParams* txDone);

    /**
     * @brief Initializes the channels masks and the channels.
     * @param [IN] params Pointer to the function parameters.
     */
    void RegionInitDefaults(RPInitDefaultsParams* params) override;

    /**
     * @brief Verifies a parameter.
     * @param [IN] verify Pointer to the function parameters.
     * @param [IN] type Sets the initialization type.
     * @retval Returns true, if the parameter is valid.
     */
    bool RegionVerify(RPVerifyParams* verify, RPPhyAttribute phyAttribute) override;

    /**
     * @brief The function parses the input buffer and sets up the channels of the
     *        CF list.
     * @param [IN] applyCFList Pointer to the function parameters.
     */
    void RegionApplyCFList(RPApplyCFListParams* applyCFList) override;

    /**
     * @brief Sets a channels mask.
     * @param [IN] chanMaskSet Pointer to the function parameters.
     * @retval Returns true, if the channels mask could be set.
     */
    bool RegionChanMaskSet(RPChanMaskSetParams* chanMaskSet) override;

    /**
     * @brief Configuration of the RX windows.
     * @param [IN] rxConfig Pointer to the function parameters.
     * @param [OUT] datarate The datarate index which was set.
     * @retval Returns true, if the configuration was applied successfully.
     */
    bool RegionRxConfig(RPRxConfigParams* rxConfig, int8_t* datarate) override;

 /**
     * Computes the Rx window timeout and offset.
     * @param [IN] datarate     Rx window datarate index to be used
     * @param [IN] minRxSymbols Minimum required number of symbols to detect an Rx frame.
     * @param [IN] rxError      System maximum timing error of the receiver. In milliseconds
     *                          The receiver will turn on in a [-rxError : +rxError] ms
     *                          interval around RxOffset
     * @param [OUT]rxConfigParams Returns updated WindowTimeout and WindowOffset fields.
     */
    void RegionComputeRxWindowParameters(int8_t datarate, uint8_t minRxSymbols, uint32_t rxError, RPRxConfigParams *rxConfigParams) override;

    /**
     * @brief TX configuration.
     * @param [IN] txConfig Pointer to the function parameters.
     * @param [OUT] txPower The tx power index which was set.
     * @param [OUT] txTimeOnAir The time-on-air of the frame.
     * @retval Returns true, if the configuration was applied successfully.
     */
    bool RegionTxConfig(RPTxConfigParams* txConfig, int8_t* txPower, uint32_t* txTimeOnAir) override;

    /**
     * @brief The function processes a Link ADR Request.
     * @param [IN] linkAdrReq Pointer to the function parameters.
     * @param [OUT] drOut The datarate which was applied.
     * @param [OUT] txPowOut The TX power which was applied.
     * @param [OUT] nbRepOut The number of repetitions to apply.
     * @param [OUT] nbBytesParsed The number bytes which were parsed.
     * @retval Returns the status of the operation, according to the LoRaMAC specification.
     */
    uint8_t RegionLinkAdrReq(RPLinkAdrReqParams* linkAdrReq, int8_t* drOut, int8_t* txPowOut, uint8_t* nbRepOut, uint8_t* nbBytesParsed) override;

    /**
     * @brief The function processes a RX Parameter Setup Request.
     * @param [IN] rxParamSetupReq Pointer to the function parameters.
     * @retval Returns the status of the operation, according to the LoRaMAC specification.
     */
    uint8_t RegionRxParamSetupReq(RPRxParamSetupReqParams* rxParamSetupReq) override;

    /**
     * @brief The function processes a New Channel Request.
     * @param [IN] newChannelReq Pointer to the function parameters.
     * @retval Returns the status of the operation, according to the LoRaMAC specification.
     */
    int8_t RegionNewChannelReq(RPNewChannelReqParams* newChannelReq) override;

    /**
     * @brief The function processes a TX ParamSetup Request.
     * @param [IN] txParamSetupReq Pointer to the function parameters.
     * @retval Returns the status of the operation, according to the LoRaMAC specification.
     *         Returns -1, if the functionality is not implemented. In this case, the end node
     *         shall ignore the command.
     */
    int8_t RegionTxParamSetupReq(RPTxParamSetupReqParams* txParamSetupReq) override;

    /**
     * @brief The function processes a DlChannel Request.
     * @param [IN] dlChannelReq Pointer to the function parameters.
     * @retval Returns the status of the operation, according to the LoRaMAC specification.
     */
    int8_t RegionDlChannelReq(RPDlChannelReqParams* dlChannelReq) override;

    /**
     * @brief Alternates the datarate of the channel for the join request.
     * @param [IN] currentDr Current datarate.
     * @param [IN] type Alternation type.
     * @retval Datarate to apply.
     */
    int8_t RegionAlternateDr(int8_t currentDr, RPAlternateDrType type) override;

    /**
     * @brief Searches and set the next random available channel
     * @param [OUT] channel Next channel to use for TX.
     * @param [OUT] time Time to wait for the next transmission according to the duty
     *              cycle.
     * @param [OUT] aggregatedTimeOff Updates the aggregated time off.
     * @retval Function status [true: OK, false: Unable to find a channel on the current datarate].
     */
    bool RegionNextChannel(RPNextChanParams* nextChanParams, uint8_t* channel, uint32_t* time, uint32_t* aggregatedTimeOff) override;

    /**
     * @brief Adds a channel.
     * @param [IN] channelAdd Pointer to the function parameters.
     * @retval Status of the operation.
     */
    bool RegionChannelAdd(RPChannelAddParams* channelAdd) override;

    /**
     * @brief Removes a channel.
     * @param [IN] channelRemove Pointer to the function parameters.
     * @retval Returns true, if the channel was removed successfully.
     */
    bool RegionChannelsRemove(RPChannelRemoveParams* channelRemove) override;

    /**
     * @brief Computes new datarate according to the given offset
     * @param [IN] downlinkDwellTime Downlink dwell time configuration. 0: No limit, 1: 400ms
     * @param [IN] dr Current datarate
     * @param [IN] drOffset Offset to be applied
     * @retval newDr Computed datarate.
     */
    uint8_t RegionApplyDrOffset(uint8_t downlinkDwellTime, int8_t dr, int8_t drOffset) override;

    /**
     * @brief Sets the radio into beacon reception mode
     * @param [IN] rxBeaconSetup Pointer to the function parameters
     * @param [out] outDr Datarate used to receive the beacon
     */
    void RegionRxBeaconSetup(RPRxBeaconSetup* rxBeaconSetup, uint8_t* outDr) override;
};
