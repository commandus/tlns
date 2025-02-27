/**
 * LoRaWAN RAK2287 gateway backend
 */
#include <iostream>
#include <cstring>
#include <thread>
#include <mutex>
#include <csignal>

#include "lorawan/lorawan-error.h"
#include "gateway-lora.h"
#include "gen/gateway-usb-conf.h"

std::string hexString(const void *buffer, size_t size);

static int getConfigForCOMPortAndRegion(
    const std::string &usbComDevicePath,
    int regionIndex
)
{
    auto &c = lorawanGatewaySettings[regionIndex];
    c.sx130x.boardConf.com_type = LGW_COM_USB;
    strcpy(c.sx130x.boardConf.com_path, usbComDevicePath.c_str());

    auto lastLgwCode = lgw_board_setconf(&c.sx130x.boardConf);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_BOARD_FAILED;
    if (c.sx130x.tsConf.enable) {
        lastLgwCode = lgw_ftime_setconf(&c.sx130x.tsConf);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_TIME_STAMP;
    }
    lastLgwCode = lgw_sx1261_setconf(&c.sx1261.sx1261);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_SX1261_RADIO;

    for (int i = 0; i < LGW_RF_CHAIN_NB; i++) {
        if (c.sx130x.txLut[i].size) {
            lastLgwCode = lgw_txgain_setconf(i, &c.sx130x.txLut[i]);
            if (lastLgwCode)
                return ERR_CODE_LORA_GATEWAY_CONFIGURE_TX_GAIN_LUT;
        }
    }

    for (int i = 0; i < LGW_RF_CHAIN_NB; i++) {
        lastLgwCode = lgw_rxrf_setconf(i, &c.sx130x.rfConfs[i]);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_INVALID_RADIO;
    }
    lastLgwCode = lgw_demod_setconf(&c.sx130x.demodConf);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_DEMODULATION;

    for (int i = 0; i < LGW_MULTI_NB; i++) {
        lastLgwCode = lgw_rxif_setconf(i, &c.sx130x.ifConfs[i]);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_MULTI_SF_CHANNEL;
    }
    if (c.sx130x.ifStdConf.enable) {
        lastLgwCode = lgw_rxif_setconf(8, &c.sx130x.ifStdConf);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_STD_CHANNEL;
    } else; // TODO
    if (c.sx130x.ifStdConf.enable) {
        lastLgwCode = lgw_rxif_setconf(9, &c.sx130x.ifFSKConf);
        if (lastLgwCode)
            return ERR_CODE_LORA_GATEWAY_CONFIGURE_FSK_CHANNEL;
    } else; // TODO
    lastLgwCode = lgw_debug_setconf(&c.debug);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_CONFIGURE_DEBUG;

    return CODE_OK;
}

#define NB_PKT_MAX         8
#define UPSTREAM_FETCH_DELAY_MS     10

class GwListener {
private:
    bool &stopRequest;
    std::mutex mLGW;
public:
    GwListener(bool &aStopRequest)
        : stopRequest(aStopRequest)
    {

    }
    void upstreamRunner() {
        // allocate memory for metadata fetching and processing
        struct lgw_pkt_rx_s rxpkt[NB_PKT_MAX]; // array containing inbound packets + metadata
        struct lgw_pkt_rx_s *p; // pointer on a RX metadata

        while (!stopRequest) {
            // fetch packets
            mLGW.lock();
            int nb_pkt = lgw_receive(NB_PKT_MAX, rxpkt);
            mLGW.unlock();
            if (nb_pkt == LGW_HAL_ERROR)
                return;

            // wait a short time if no packets, nor status report
            if (nb_pkt == 0) {
                wait_ms(UPSTREAM_FETCH_DELAY_MS);
                continue;
            }
            int pkt_in_dgram = 0;
            for (int i = 0; i < nb_pkt; ++i) {
                p = &rxpkt[i];
                std::cout << hexString((const char *) p, sizeof(struct lgw_pkt_rx_s)) << std::endl;
            }
        }
    }
};

static int testUpstream(
    const GatewaySettings *config
) {
    if (!config)
        return ERR_CODE_NO_CONFIG;
    bool stopRequest = false;
    int lastLgwCode = 0;
    // starting the concentrator
    lastLgwCode = lgw_start();
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_START_FAILED;

    // getUplink the concentrator EUI
    uint64_t gwEUI = 0;
    lastLgwCode = lgw_get_eui(&gwEUI);
    if (lastLgwCode)
        return ERR_CODE_LORA_GATEWAY_GET_EUI;

    GwListener listener(stopRequest);
    std::thread upstreamThread(&GwListener::upstreamRunner, &listener);
    upstreamThread.join();
    return 0;
}

static bool stopRequest = false;

void signalHandler(int signal) {
    switch (signal)
    {
        case SIGINT:
            std::cerr << "Interrupted.." << std::endl;
            stopRequest = true;
            break;
        default:
            break;
    }
}

#ifdef _MSC_VER
BOOL WINAPI winSignalHandler(DWORD signal) {
    std::cerr << "Interrupted.." << std::endl;
    stopRequest = true;
    return true;
}
#endif

    void setSignalHandler() {
#ifdef _MSC_VER
        SetConsoleCtrlHandler(winSignalHandler,  true);
#else
        struct sigaction action {};
        memset(&action, 0, sizeof(struct sigaction));
        action.sa_handler = &signalHandler;
        sigaction(SIGINT, &action, nullptr);
#endif
    }

int main(
	int argc,
	char *argv[]
)
{
    int region = 6;
    int r = getConfigForCOMPortAndRegion(argc > 1 ? argv[1] : "COM3", region);

    if (!r) {
        setSignalHandler();
        testUpstream(&lorawanGatewaySettings[region]);
    }
    return r;
}
