#include <thread>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "gw-dev/usb/usb-listener.h"

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
    << "Frequency        " << std::fixed << std::setprecision(6) << rx->freq_hz << " Hz" << DLMT
    << "Frequency offset " << rx->freq_offset << " Hz" << DLMT
    << "IF chain         " << (int) rx->if_chain << DLMT
    << "Status           " << (int) rx->status << DLMT
    << "Counter          " << rx->count_us << " microseconds" << DLMT
    << "RF chain         " << (int) rx->rf_chain << DLMT
    << "Modem            " << (int) rx->modem_id << DLMT
    << "Modulation       " << MODULATION2String((MODULATION) rx->modulation) << DLMT
    << "Bandwidth        " << DATA_RATE2string((BANDWIDTH) (rx->bandwidth + 3), (SPREADING_FACTOR) rx->datarate) << DLMT
    << "Coding date      " << codingRate2string((CODING_RATE) rx->coderate) << DLMT
    << std::fixed << std::setprecision(2)
    << "Channel RSSI     " << rx->rssic << " dB" << DLMT
    << "Signal RSSI      " << rx->rssis << " dB" << DLMT
    << "Signal/noise     " << rx->snr << " dB" << DLMT
    << "Signal/noise min " << rx->snr_min << " dB" << DLMT
    << "Signal/noise max " << rx->snr_max << " dB" << DLMT
    << "CRC              " << std::hex << std::setfill('0') << std::setw(4) << rx->crc << DLMT
    << std::dec << std::setw(0)
    << "Payload size     " << rx->size << " bytes" << DLMT
    << "Payload          " << hexString(rx->payload, rx->size) << DLMT
    << "Fine timestamp   " << (rx->ftime_received ? "true": "false") << DLMT
    << "Since last PPS   " << rx->ftime << " nanoseconds" << DLMT;

    return ss.str();
}

static std::string lgw_pkt_rx_s2json(
    const lgw_pkt_rx_s *rx
)
{
    if (!rx)
        return "{}";
    std::stringstream ss;
    ss  << "{\"freq\": " << std::fixed << std::setprecision(6) << rx->freq_hz + rx->freq_offset
        << ", \"status\": " << (int) rx->status
        << ", \"count_us\": " << rx->count_us
        << ", \"rfch\": " << (int) rx->if_chain
        << ", \"modem\": " << (int) rx->modem_id
        << R"(, "modu": ")" << MODULATION2String((MODULATION) rx->modulation)
        << R"(", "bandwidth": ")" << DATA_RATE2string((BANDWIDTH) (rx->bandwidth + 3), (SPREADING_FACTOR) rx->datarate)
        << R"(", "codr": ")" << codingRate2string((CODING_RATE) rx->coderate)
        << std::fixed << std::setprecision(2)
        << R"(", "rssic": )" << rx->rssic
        << ", \"rssis\": " << rx->rssis
        << ", \"snr\": " << rx->snr
        << ", \"snr_min\": " << rx->snr_min
        << ", \"snr_max\": " << rx->snr_max
        << R"(, "crc": ")" << std::hex << std::setfill('0') << std::setw(4) << rx->crc
        << std::dec << std::setw(0)
        << R"(", "size": )" << rx->size
        << R"(, "payload": ")" << hexString(rx->payload, rx->size)
        << R"(", "ftime_received": )" << (rx->ftime_received ? "true" : "false")
        << ", \"ftime\": " << rx->ftime
        << "}";
    return ss.str();
}

UsbListener::UsbListener()
    : UsbLoRaWANGateway(), upstreamThread(nullptr), state(USB_LISTENER_STATE_STOPPED)
{
}

/**
 * Copy constructor
 * @param value Must be stopped
 */
UsbListener::UsbListener(const UsbListener& value)
    : UsbLoRaWANGateway(value.gatewaySettings, value.eui), upstreamThread(value.upstreamThread), state(value.state)
{
}


UsbListener::~UsbListener()
{
    stop(true);
}

int UsbListener::start()
{
    if (state == USB_LISTENER_STATE_RUNNING)
        return 0;
    state = USB_LISTENER_STATE_RUNNING;
    // starting the concentrator
    int r = lgw_start();
    if (r)
        return ERR_CODE_LORA_GATEWAY_START_FAILED;

    // get the concentrator EUI
    r = lgw_get_eui(&eui);
    if (r)
        return ERR_CODE_LORA_GATEWAY_GET_EUI;
    upstreamThread = new std::thread(&UsbListener::runner, this);
    return 0;
}

int UsbListener::runner()
{
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
        for (int i = 0; i < nb_pkt; ++i) {
            auto p = &rxpkt[i];
            std::cout << time2string(time(nullptr)) << '\n' << lgw_pkt_rx_s2string(p) << std::endl;
            std::cout << lgw_pkt_rx_s2json(p) << std::endl;
            printRFM2json(std::cout, p->payload, p->size);
            std::cout << std::endl;
        }
    }
    state = USB_LISTENER_STATE_STOPPED;
    return 0;
}

int UsbListener::stop(
    bool aWait
)
{
    if (state == USB_LISTENER_STATE_STOPPED)
        return 0;
    state = USB_LISTENER_STATE_STOP_REQUEST;
    if (aWait)
        wait();
    upstreamThread = nullptr;
    return 0;
}

void UsbListener::wait() {
    if (!upstreamThread)
        return;
    upstreamThread->join();
}
