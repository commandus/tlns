#ifndef REGIONAL_PARAMETERS_AUTOGEN_H
#define REGIONAL_PARAMETERS_AUTOGEN_H

/**
 *
 * Autogenerated file. Do not modify.
 *
 * Generated by:
 *      ./regional-parameters2cpp regional-parameters.json 
 * on 2025-03-14T19:21:46+0900
 *
 */

#include "lorawan/regional-parameters/regional-parameter-channel-plan-mem.h"

RegionalParameterChannelPlanMem regionalParameterChannelPlanMem(
	{
		// file regional-parameters.json
		{
			1, // id
			"EU863-870", // name
			"EU868", // cn 
			16.00f, // maxUplinkEIRP
			14, // defaultDownlinkTXPower
			869525000, // pingSlotFrequency
			false, // implementsTXParamSetup
			false, // defaultRegion
			true, // supportsExtraChannels
			BandDefaults({ // bandDefaults
				869100000, // RX2Frequency
				0, // RX2DataRate
				1, // ReceiveDelay1
				5, // JoinAcceptDelay1
				6 // JoinAcceptDelay2
			}),
			{ // dataRates
				DataRate({
					true, // uplink
					true, // downlink
					(MODULATION) 16, // modulation
					(BANDWIDTH) 7, // bandwidth
					(SPREADING_FACTOR) 12, // spreadingFactor
					250 // bps
				}),
				DataRate({
					true, // uplink
					true, // downlink
					(MODULATION) 16, // modulation
					(BANDWIDTH) 7, // bandwidth
					(SPREADING_FACTOR) 11, // spreadingFactor
					440 // bps
				}),
				DataRate({
					true, // uplink
					true, // downlink
					(MODULATION) 16, // modulation
					(BANDWIDTH) 7, // bandwidth
					(SPREADING_FACTOR) 10, // spreadingFactor
					980 // bps
				}),
				DataRate({
					true, // uplink
					true, // downlink
					(MODULATION) 16, // modulation
					(BANDWIDTH) 7, // bandwidth
					(SPREADING_FACTOR) 9, // spreadingFactor
					1760 // bps
				}),
				DataRate({
					true, // uplink
					true, // downlink
					(MODULATION) 16, // modulation
					(BANDWIDTH) 7, // bandwidth
					(SPREADING_FACTOR) 8, // spreadingFactor
					3125 // bps
				}),
				DataRate({
					true, // uplink
					true, // downlink
					(MODULATION) 16, // modulation
					(BANDWIDTH) 7, // bandwidth
					(SPREADING_FACTOR) 7, // spreadingFactor
					5470 // bps
				}),
				DataRate({
					true, // uplink
					true, // downlink
					(MODULATION) 16, // modulation
					(BANDWIDTH) 8, // bandwidth
					(SPREADING_FACTOR) 7, // spreadingFactor
					11000 // bps
				}),
				DataRate({
					true, // uplink
					true, // downlink
					(MODULATION) 32, // modulation
					(BANDWIDTH) 0, // bandwidth
					(SPREADING_FACTOR) 0, // spreadingFactor
					50000 // bps
				})
			},
			{ // maxPayloadSizePerDataRate
				MaxPayloadSize({
					59, // m
					51 // n
				}),
				MaxPayloadSize({
					59, // m
					51 // n
				}),
				MaxPayloadSize({
					59, // m
					51 // n
				}),
				MaxPayloadSize({
					123, // m
					115 // n
				}),
				MaxPayloadSize({
					230, // m
					222 // n
				}),
				MaxPayloadSize({
					230, // m
					222 // n
				}),
				MaxPayloadSize({
					230, // m
					222 // n
				}),
				MaxPayloadSize({
					230, // m
					222 // n
				})
			},
			{ // maxPayloadSizePerDataRateRepeater
				MaxPayloadSize({
					0, // m
					0 // n
				}),
				MaxPayloadSize({
					0, // m
					0 // n
				}),
				MaxPayloadSize({
					0, // m
					0 // n
				}),
				MaxPayloadSize({
					0, // m
					0 // n
				}),
				MaxPayloadSize({
					0, // m
					0 // n
				}),
				MaxPayloadSize({
					0, // m
					0 // n
				}),
				MaxPayloadSize({
					0, // m
					0 // n
				}),
				MaxPayloadSize({
					0, // m
					0 // n
				})
			},
			// rx1DataRateOffsets
			{{}, {}, {}, {}, {}, {}, {}, {}},
			// txPowerOffsets
			{},
			// uplinkChannels
			{
				Channel({
					868900000, // frequency
					0, // minDR
					5, // maxDR
					true, // enabled
					false // custom
				}),
				Channel({
					869100000, // frequency
					0, // minDR
					5, // maxDR
					true, // enabled
					false // custom
				})
			},
			{ // downlinkChannels
				Channel({
					868900000, // frequency
					0, // minDR
					5, // maxDR
					true, // enabled
					false // custom
				}),
				Channel({
					869100000, // frequency
					0, // minDR
					5, // maxDR
					true, // enabled
					false // custom
				})
			}
		}
	}
);
#endif
