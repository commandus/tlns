#include "lorawan/proto/gw/set-gateway-metadata.h"

#define DEF_WINDOW_1_MICROSECONDS    1000000

/**
 * Invalidate frequency and other regional settings.
 * @param val value to fix
 * @param gatewaySettings data to check with
 */
void invalidateSEMTECH_PROTOCOL_METADATA_TX(
    SEMTECH_PROTOCOL_METADATA_TX &val,
    const GatewaySettings &gatewaySettings
)
{
    // RF channel
    if (val.rf_chain >= LGW_RF_CHAIN_NB)
        val.rf_chain = 0;
    // RF channel frequency
    if (val.freq_hz > gatewaySettings.sx130x.tx_freq_max[val.rf_chain])
        val.freq_hz = gatewaySettings.sx130x.tx_freq_max[val.rf_chain];
    else
        if (val.freq_hz < gatewaySettings.sx130x.tx_freq_min[val.rf_chain])
            val.freq_hz = gatewaySettings.sx130x.tx_freq_min[val.rf_chain];
    // check other settings if required
}

static int8_t calculateTxPower(
    const SEMTECH_PROTOCOL_METADATA_RX &metadata,
    const GatewaySettings &gatewaySettings,
    uint16_t payloadSize
) {
    int power = (int) gatewaySettings.regionalParameterChannelPlan->maxUplinkEIRP; //defaultDownlinkTXPower;
    return 0;
}

/**
 * Setup metadata for transmission gateway
 * @param retVal
 * @param gatewaySettings
 */
void setSEMTECH_PROTOCOL_METADATA_TX(
    SEMTECH_PROTOCOL_METADATA_TX &retVal,
    const GatewaySettings &gatewaySettings,
    const SEMTECH_PROTOCOL_METADATA_RX &rxMetadata,
    uint16_t payloadSize
)
{
    retVal.freq_hz = rxMetadata.freq;               // uint32_t center frequency of
    retVal.tx_mode = 1;                             // uint8_t select on what event/time the TX is triggered IMMEDIATE 0, TIMESTAMPED 1, ON_GPS 2
    retVal.count_us = rxMetadata.tmst + DEF_WINDOW_1_MICROSECONDS;    // uint32_t timestamp or delay in microseconds for TX trigger
    retVal.rf_chain = rxMetadata.rfch;              // uint8_t through which RF chain will the packet be sent
    retVal.rf_power = calculateTxPower(rxMetadata, gatewaySettings, payloadSize); // int8_t TX power, in dBm
    retVal.modulation = rxMetadata.modu;            // uint8_t modulation to use for the packet
    retVal.bandwidth = rxMetadata.bandwidth;        // uint8_t modulation bandwidth (LoRa only)
    retVal.datarate = rxMetadata.spreadingFactor;   // uint32_t TX datarate (baudrate for MODULATION_FSK, SF for LoRa)
    retVal.coderate = rxMetadata.codingRate;        // uint8_t error-correcting code of the packet (LoRa only)
    retVal.invert_pol = true;                       // bool invert signal polarity, for orthogonal downlinks (LoRa only)
    retVal.f_dev = 0;                               // uint8_t frequency deviation, in kHz (MODULATION_FSK only)
    retVal.preamble = 0;                            // uint16_t set the preamble length, 0 for default
    retVal.no_crc = false;                          // bool if true, do not send a CRC in the packet
    retVal.no_header = false;                       // bool if true, enable implicit header mode (LoRa), fixed length (MODULATION_FSK)
    retVal.size = payloadSize;                      // uint16_t payload size in bytes
    invalidateSEMTECH_PROTOCOL_METADATA_TX(retVal, gatewaySettings);
}
