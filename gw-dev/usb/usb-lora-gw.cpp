#include "gw-dev/usb/usb-lora-gw.h"

#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"

// ms waited when a fetch return no packets
UsbLoRaWANGateway::UsbLoRaWANGateway()
    : gatewaySettings(nullptr), eui(0), enable(true)
{
}


UsbLoRaWANGateway::UsbLoRaWANGateway(
    GatewaySettings *aGatewaySettings,
    uint64_t aEui
)
    : gatewaySettings(aGatewaySettings), eui(aEui), enable(true)
{
}

/**
 * Copy constructor
 * @param value Must be stopped
 */
UsbLoRaWANGateway::UsbLoRaWANGateway(const UsbLoRaWANGateway& value)
    : gatewaySettings(value.gatewaySettings), eui(value.eui), enable(value.enable)
{
}

int UsbLoRaWANGateway::init(
    GatewaySettings *aGatewaySettings
)
{
    gatewaySettings = aGatewaySettings;

    int lastLgwCode;
    if (!gatewaySettings)
        return ERR_CODE_INSUFFICIENT_PARAMS;
    lastLgwCode = lgw_board_setconf(&gatewaySettings->sx130x.boardConf);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_BOARD_FAILED;
    if (gatewaySettings->sx130x.tsConf.enable) {
        lastLgwCode = lgw_ftime_setconf(&gatewaySettings->sx130x.tsConf);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_TIME_STAMP;
    }
    lastLgwCode = lgw_sx1261_setconf(&gatewaySettings->sx1261.sx1261);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_SX1261_RADIO;

    for (int i = 0; i < LGW_RF_CHAIN_NB; i++) {
        if (gatewaySettings->sx130x.txLut[i].size) {
            lastLgwCode = lgw_txgain_setconf(i, &gatewaySettings->sx130x.txLut[i]);
            if (lastLgwCode)
                return ERR_CODE_LORA_GATEWAY_CONFIGURE_TX_GAIN_LUT;
        }
    }

    for (int i = 0; i < LGW_RF_CHAIN_NB; i++) {
        lastLgwCode = lgw_rxrf_setconf(i, &gatewaySettings->sx130x.rfConfs[i]);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_INVALID_RADIO;
    }
    lastLgwCode = lgw_demod_setconf(&gatewaySettings->sx130x.demodConf);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_DEMODULATION;

    for (int i = 0; i < LGW_MULTI_NB; i++) {
        lastLgwCode = lgw_rxif_setconf(i, &gatewaySettings->sx130x.ifConfs[i]);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_MULTI_SF_CHANNEL;
    }
    if (gatewaySettings->sx130x.ifStdConf.enable) {
        lastLgwCode = lgw_rxif_setconf(8, &gatewaySettings->sx130x.ifStdConf);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_STD_CHANNEL;
    }
    if (gatewaySettings->sx130x.ifStdConf.enable) {
        lastLgwCode = lgw_rxif_setconf(9, &gatewaySettings->sx130x.ifFSKConf);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_FSK_CHANNEL;
    }
    lastLgwCode = lgw_debug_setconf(&gatewaySettings->debug);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_DEBUG;
    return CODE_OK;
}

UsbLoRaWANGateway::~UsbLoRaWANGateway()
{
}
