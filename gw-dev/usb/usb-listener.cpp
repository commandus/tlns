#include <thread>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "usb-listener.h"
#include "loragw_hal.h"

#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-date.h"
#include "lorawan/lorawan-string.h"

// max number of packets per fetch/send cycle
#define NB_PKT_MAX         255
// ms waited when a fetch return no packets
#define UPSTREAM_FETCH_DELAY_MS     10

static const char DLMT = '\n';

static std::string lgw_pkt_rx_s2string(
    const lgw_pkt_rx_s *rx
)
{
    if (!rx)
        return "";
    std::stringstream ss;
    ss
    << "Frequency        " << rx->freq_hz << " Hz" << DLMT
    << "Frequency offset " << rx->freq_offset << " Hz" << DLMT
    << "IF chain         " << rx->if_chain << " Hz" << DLMT
    << "Status           " << (int) rx->status << DLMT
    << "Counter          " << rx->count_us << " microseconds" << DLMT
    << "RF chain         " << (int) rx->rf_chain << DLMT
    << "Modem            " << (int) rx->modem_id << DLMT
    << "Modulation       " << MODULATION2String((MODULATION) rx->modulation) << DLMT
    << "Bandwidth        " << DATA_RATE2string((BANDWIDTH) rx->bandwidth, (SPREADING_FACTOR) rx->datarate) << DLMT
    << "Coding date      " << codingRate2string((CODING_RATE) rx->coderate) << DLMT
    << std::fixed << std::setprecision(2)
    << "Channel RSSI     " << rx->rssic << " dB" << DLMT
    << "Signal RSSI      " << rx->rssis << " dB" << DLMT
    << "Signal/noise     " << rx->snr << " dB" << DLMT
    << "Signal/noise min " << rx->snr_min << " dB" << DLMT
    << "Signal/noise max " << rx->snr_max << " dB" << DLMT
    << std::hex << std::setfill('0') << std::setw(4)
    << "CRC              " << rx->crc << DLMT
    << std::dec << std::setw(0)
    << "Payload size     " << rx->size << " bytes" << DLMT
    << "Payload          " << hexString(rx->payload, rx->size) << " bytes" << DLMT
    << "Fine timestamp   " << (rx->ftime_received ? "true": "false") << DLMT
    << "Since last PPS   " << rx->ftime << " nanoseconds" << DLMT;
    return ss.str();
}

UsbListener::UsbListener()
    : gatewaySettings(nullptr), state(USB_LISTENER_STATE_STOPPED)
{

}

UsbListener::UsbListener(
    GatewaySettings *aGatewaySettings
)
    : gatewaySettings(nullptr), state(USB_LISTENER_STATE_STOPPED)
{
    init(aGatewaySettings);
}

/**
 * Copy constructor
 * @param value Must be stopped
 */
UsbListener::UsbListener(const UsbListener& value)
    : gatewaySettings(value.gatewaySettings), state(value.state)
{
}

int UsbListener::init(
    GatewaySettings *aGatewaySettings
)
{
    stop();
    gatewaySettings = aGatewaySettings;

    int lastLgwCode = 0;
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
    } else; // TODO
    if (gatewaySettings->sx130x.ifStdConf.enable) {
        lastLgwCode = lgw_rxif_setconf(9, &gatewaySettings->sx130x.ifFSKConf);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_FSK_CHANNEL;
    } else; // TODO
    lastLgwCode = lgw_debug_setconf(&gatewaySettings->debug);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_DEBUG;
    return CODE_OK;
}

UsbListener::~UsbListener()
{
    stop();
}

int UsbListener::start()
{
    if (state == USB_LISTENER_STATE_RUNNING)
        return 0;
    std::thread upstreamThread(&UsbListener::runner, this);
    upstreamThread.detach();
    return 0;
}

int UsbListener::runner()
{
    state = USB_LISTENER_STATE_RUNNING;
    struct lgw_pkt_rx_s rxpkt[NB_PKT_MAX]; // array containing inbound packets + metadata
    while (state == USB_LISTENER_STATE_RUNNING) {
        // fetch packets
        int nb_pkt = lgw_receive(NB_PKT_MAX, rxpkt);
        if (nb_pkt == LGW_HAL_ERROR) {
            return ERR_CODE_LORA_GATEWAY_FETCH;
        }

        // wait a short time if no packets, nor status report
        if (nb_pkt == 0) {
            wait_ms(UPSTREAM_FETCH_DELAY_MS);
            continue;
        }

        // serialize Lora packets metadata and payload
        int pkt_in_dgram = 0;
        for (int i = 0; i < nb_pkt; ++i) {
            auto p = &rxpkt[i];
            std::cout << time2string(time(nullptr)) << ' ' << lgw_pkt_rx_s2string(p) << std::endl;
        }
    }
    std::unique_lock<std::mutex> lck(mutexState);
    state = USB_LISTENER_STATE_STOPPED;
    cvState.notify_all();
    return 0;
}

int UsbListener::stop()
{
    if (state == USB_LISTENER_STATE_STOPPED)
        return 0;
    state = USB_LISTENER_STATE_STOP_REQUEST;
    // wait until thread finished
    std::unique_lock<std::mutex> lock(mutexState);
    while (state != USB_LISTENER_STATE_STOPPED)
        cvState.wait(lock);
}
