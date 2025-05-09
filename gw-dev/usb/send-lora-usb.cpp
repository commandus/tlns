/**
 * Example send LoRaWAN packet using RAK2287 USB stick
 */
#include <iostream>
#include <cstring>
#include <csignal>

#if defined(_MSC_VER) || defined(__MINGW32__)
#else
#include <execinfo.h>
#include <sys/un.h>
#endif

#include "argtable3/argtable3.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-msg.h"
#include "lorawan/helper/file-helper.h"

#include "task-usb-socket.h"
#include "lorawan/lorawan-string.h"
#include "gateway-settings-helper.h"
#include "gw-dev/usb/usb-lora-gw.h"

// generated gateway regional settings source code
#include "gen/gateway-usb-conf.h"
#include "gen/regional-parameters-3.h"

// i18n
// #include <libintl.h>
// #define _(String) gettext (String)
#define _(String) (String)

const std::string programName = _("send-lora-usb");

#define STD_LORA_PREAMBLE           8
#define STD_FSK_PREAMBLE            5

static int getDEVADDR(
    DEVADDR &retVal,
    const uint8_t *payload,
    uint16_t size
) {
    if (size < SIZE_MHDR + SIZE_FHDR)
        return ERR_CODE_INVALID_PACKET;
    auto *f = (FHDR*) (payload + SIZE_MHDR);
    retVal = f->devaddr;
    return CODE_OK;
}

class LocalSenderConfiguration {
public:
    std::vector <UsbLoRaWANGateway> gateway;
    std::vector <std::string> devicePaths;
    size_t regionGWIdx;
    bool rx1;
    bool rx2;
    uint8_t rx1dataRateOffset;
    bool classC;
    int verbosity;
    std::string payload;
    bool stopRequest;
    const RegionalParameterChannelPlan *channelPlan;
    LocalSenderConfiguration()
        : regionGWIdx(0), rx1(false), rx2(false),
          rx1dataRateOffset(0), classC(true), verbosity(0), stopRequest(false), channelPlan(nullptr)
    {
    }
};

static bool waitSent(
    uint8_t rf_chain,
    int timeoutSeconds
) {
    bool sent = false;
    time_t t;
    time_t t2;
    time(&t);
    t2 = t;
    while (t + timeoutSeconds < t2) {
        uint8_t c;
        int r = lgw_status(rf_chain, TX_STATUS, &c);
        if (r)
            break;
        if (c == TX_EMITTING) {
            sent = true;
            continue;
        }
        if (sent && c == TX_FREE)
            break;
        time(&t2);
    }
    return sent ? CODE_OK : ERR_CODE_LORA_GATEWAY_SEND_FAILED;
}

static LocalSenderConfiguration localConfig;

static void stop()
{
    localConfig.stopRequest = true;
}

static void done()
{
    localConfig.gateway.clear();
}

/**
 * Parse command line
 * Return 0- success
 *        ERR_CODE_PARAM_INVALID- show help and exit, or command syntax error
 **/
int parseCmd(
    LocalSenderConfiguration *config,
    int argc,
    char *argv[]
)
{
    // device path
    struct arg_str *a_device_path = arg_strn(nullptr, nullptr, _("<device-name>"), 1, 1, _("USB gateway device e.g. /dev/ttyACM0"));
    struct arg_str *a_region_name = arg_str1("c", "region", _("<region-name>"), _("Region name, e.g. \"EU433\" or \"US\""));
    struct arg_str *a_payload = arg_str1("p", "payload", _("<hex-string>"), _("Radio packet, hex string, e.g. 60e26a7e00000000026b69636b6173732d776f7a6e69616b5a54167c"));
    struct arg_lit *a_rx1 = arg_lit0("1", "rx1", _("Device is class A. Wait downlink message then send uplink message in RX1 window"));
    struct arg_lit *a_rx2 = arg_lit0("2", "rx2", _("Device is class A. Wait downlink message then send uplink message in RX2 window"));
    struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 7, _("Verbosity level 1- alert, 2-critical error, 3- error, 4- warning, 5- siginicant info, 6- info, 7- debug"));
    struct arg_lit *a_help = arg_lit0("?", "help", _("Show this help"));
    struct arg_end *a_end = arg_end(20);

    void *argtable[] = {
    a_device_path, a_region_name, a_payload, a_rx1, a_rx2, a_verbosity, a_help, a_end
    };

    // verify the argtable[] entries were allocated successfully
    if (arg_nullcheck(argtable) != 0) {
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return ERR_CODE_PARAM_INVALID;
    }
    // Parse the command line as defined by argtable[]
    int nErrors = arg_parse(argc, argv, argtable);

    for (int i = 0; i < a_device_path->count; i++)
        config->devicePaths.emplace_back(a_device_path->sval[i]);

    if (a_region_name->count) {
        config->regionGWIdx = findGatewayRegionIndex(lorawanGatewaySettings, *a_region_name->sval);
        config->channelPlan = regionalParameterChannelPlanMem.get(*a_region_name->sval);
    } else {
        config->regionGWIdx = 0;
        config->channelPlan = regionalParameterChannelPlanMem.get(0);
    }

    if (a_payload->count)
        config->payload = hex2string(a_payload->sval[0]);

    config->rx1 = a_rx1->count > 0;
    config->rx2 = a_rx2->count > 0;
    config->classC = !(config->rx1 || config->rx2);
    config->verbosity = a_verbosity->count;

    // special case: '--help' takes precedence over error reporting
    if ((a_help->count) || nErrors) {
        if (nErrors)
            arg_print_errors(stderr, a_end, programName.c_str());
        std::cerr << _("Usage: ") << programName << std::endl;
        arg_print_syntax(stderr, argtable, "\n");
        std::cerr << MSG_PROG_NAME_LISTEN_LORA_USB << std::endl;
        arg_print_glossary(stderr, argtable, "  %-25s %s\n");
        std::cerr << _("  region name: ");
        for (auto & lorawanGatewaySetting : lorawanGatewaySettings)
            std::cerr << "\"" << lorawanGatewaySetting.name << "\" ";
        std::cerr << std::endl;
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return ERR_CODE_PARAM_INVALID;
    }

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return CODE_OK;
}

#ifdef _MSC_VER
#undef ENABLE_TERM_COLOR
#else
#define ENABLE_TERM_COLOR	1
#endif

#define TRACE_BUFFER_SIZE   256

static void printTrace() {
#ifdef _MSC_VER
#else
    void *t[TRACE_BUFFER_SIZE];
    auto size = backtrace(t, TRACE_BUFFER_SIZE);
    backtrace_symbols_fd(t, size, STDERR_FILENO);
#endif
}

#ifndef _MSC_VER
static void run();
void signalHandler(int signal)
{
    switch (signal) {
        case SIGINT:
            std::cerr << MSG_INTERRUPTED << std::endl;
            stop();
            done();
            std::cerr << MSG_GRACEFULLY_STOPPED << std::endl;
            exit(CODE_OK);
        case SIGSEGV:
            std::cerr << ERR_SEGMENTATION_FAULT << std::endl;
            printTrace();
            exit(ERR_CODE_SEGMENTATION_FAULT);
        case SIGABRT:
            std::cerr << ERR_ABRT << std::endl;
            printTrace();
            exit(ERR_CODE_ABRT);
        case SIGHUP:
            std::cerr << ERR_HANGUP_DETECTED << std::endl;
            break;
        case SIGUSR2:	// 12
            std::cerr << MSG_SIG_FLUSH_FILES << std::endl;
            // flushFiles();
            break;
        case 42:	// restart
            std::cerr << MSG_RESTART_REQUEST << std::endl;
            stop();
            done();
            run();
            break;
        default:
            break;
    }
}
#else
BOOL WINAPI winSignalHandler(DWORD signal) {
    std::cerr << "Interrupted.." << std::endl;
    stop();
    done();
    std::cerr << MSG_GRACEFULLY_STOPPED << std::endl;
    return true;
}
#endif

void setSignalHandler()
{
#ifdef _MSC_VER
    SetConsoleCtrlHandler(winSignalHandler,  true);
#else
    struct sigaction action {};
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = &signalHandler;
    sigaction(SIGINT, &action, nullptr);
    sigaction(SIGHUP, &action, nullptr);
    sigaction(SIGSEGV, &action, nullptr);
    sigaction(SIGABRT, &action, nullptr);
    sigaction(SIGUSR2, &action, nullptr);
    sigaction(42, &action, nullptr);
#endif
}

#define RECEIVE_DELAY1 1000000
#define RECEIVE_DELAY2 2000000

static int sendClassARx1(
    lgw_pkt_rx_s &rx
) {
    struct lgw_pkt_tx_s pkt{};
    if (!localConfig.channelPlan)
        return ERR_CODE_PARAM_INVALID;
    pkt.rf_chain = rx.rf_chain;
    if (!lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.rfConfs[pkt.rf_chain].enable
        || !lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.rfConfs[pkt.rf_chain].tx_enable) {
        // RF chain disabled or is not intended for transmission
        return ERR_CODE_PARAM_INVALID;
    }
    pkt.freq_hz = rx.freq_hz;
    if ((pkt.freq_hz < lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.tx_freq_min[pkt.rf_chain]) ||
        pkt.freq_hz > lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.tx_freq_max[pkt.rf_chain]) {
        // unsupported frequency
        return ERR_CODE_PARAM_INVALID;
    }
    pkt.freq_offset = 0;                    // frequency offset from Radio Tx frequency (CW mode)
    pkt.tx_mode = TIMESTAMPED;              // sent at time stamp
    pkt.count_us = rx.count_us + localConfig.channelPlan->value.bandDefaults.value.ReceiveDelay1 * 1000000;

    // get data rate
    pkt.datarate = rx.datarate;
    // shift data rate
    if (rx.datarate < DATA_RATE_SIZE && localConfig.rx1dataRateOffset < localConfig.channelPlan->value.rx1DataRateOffsets[rx.datarate].size())
        pkt.datarate = localConfig.channelPlan->value.rx1DataRateOffsets[rx.datarate][localConfig.rx1dataRateOffset];

    bool payloadIsDownlink = isDownlink(localConfig.payload.c_str(), localConfig.payload.size());
    pkt.rf_power = lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.txLut[pkt.rf_chain].lut[0].rf_power;   // int8_t TX power, in dBm
    pkt.modulation = MODULATION_LORA;       // uint8_t modulation to use for the packet
    pkt.bandwidth = BW_125KHZ;              // uint8_t modulation bandwidth (LoRa only)
    pkt.coderate = CRLORA_4_5;              // uint8_t error-correcting code of the packet (LoRa only)
    pkt.invert_pol = payloadIsDownlink;     // bool invert signal polarity, for orthogonal downlinks (LoRa only)
    pkt.f_dev = 0;                          // uint8_t frequency deviation, in kHz (FSK only)
    pkt.preamble = STD_LORA_PREAMBLE;       // uint16_t set the preamble length, 0 for default

    pkt.size = (uint16_t) localConfig.payload.size();   // uint16_t payload size in bytes
    memmove(pkt.payload, localConfig.payload.c_str(), localConfig.payload.size());

    int r = lgw_send(&pkt);
    if (r)
        return ERR_CODE_LORA_GATEWAY_SEND_FAILED;
    waitSent(pkt.rf_chain, 2);
    return CODE_OK;
}

static int sendClassARx2(
    lgw_pkt_rx_s &rx
) {
    struct lgw_pkt_tx_s pkt{};
    // get radio channel chain
    int rfChain = -1;
    for (int c = 0; c < LGW_RF_CHAIN_NB; c++) {
        if (lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.rfConfs[c].enable
            && lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.rfConfs[c].tx_enable) {
            rfChain = c;
            break;
        }
    }
    if (rfChain < 0)
        return ERR_CODE_PARAM_INVALID;
    pkt.rf_chain = rfChain;

    // set frequency & data rate
    if (!localConfig.channelPlan)
        return ERR_CODE_PARAM_INVALID;
    pkt.freq_hz = localConfig.channelPlan->value.bandDefaults.value.RX2Frequency;
    pkt.datarate = 12 - localConfig.channelPlan->value.bandDefaults.value.RX2DataRate;  // data rate 0 -> spreading factor 12 (~250bit/s)
    pkt.count_us = rx.count_us + (localConfig.channelPlan->value.bandDefaults.value.ReceiveDelay2 * 1000000);

    // validate frequency does gateway support it
    if ((pkt.freq_hz < lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.tx_freq_min[pkt.rf_chain]) ||
        pkt.freq_hz > lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.tx_freq_max[pkt.rf_chain]) {
        // unsupported frequency
        return ERR_CODE_PARAM_INVALID;
    }

    if (!lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.rfConfs[pkt.rf_chain].enable
        || !lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.rfConfs[pkt.rf_chain].tx_enable) {
        // RF chain disabled or is not intended for transmission
        return ERR_CODE_PARAM_INVALID;
    }

    bool payloadIsDownlink = isDownlink(localConfig.payload.c_str(), localConfig.payload.size());
    pkt.tx_mode = TIMESTAMPED;                // immediately uint8_t select on what event/time the TX is triggered
    pkt.rf_power = lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.txLut[pkt.rf_chain].lut[0].rf_power;   // int8_t TX power, in dBm
    pkt.modulation = MODULATION_LORA;       // uint8_t modulation to use for the packet
    pkt.freq_offset = 0;                    // frequency offset from Radio Tx frequency (CW mode)
    pkt.bandwidth = BW_125KHZ;              // uint8_t modulation bandwidth (LoRa only)
    pkt.coderate = CRLORA_4_5;              // uint8_t error-correcting code of the packet (LoRa only)
    pkt.invert_pol = payloadIsDownlink;     // bool invert signal polarity, for orthogonal downlinks (LoRa only)
    pkt.f_dev = 0;                          // uint8_t frequency deviation, in kHz (FSK only)
    pkt.preamble = STD_LORA_PREAMBLE;       // uint16_t set the preamble length, 0 for default

    pkt.size = (uint16_t) localConfig.payload.size();   // uint16_t payload size in bytes
    memmove(pkt.payload, localConfig.payload.c_str(), localConfig.payload.size());

    int r = lgw_send(&pkt);
    if (r)
        return ERR_CODE_LORA_GATEWAY_SEND_FAILED;
    waitSent(pkt.rf_chain, 3);
    return CODE_OK;
}

static int sendClassC(
    UsbLoRaWANGateway &gateway
) {
    struct lgw_pkt_tx_s pkt{};
    int rfChain = -1;
    for (int c = 0; c < LGW_RF_CHAIN_NB; c++) {
        if (lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.rfConfs[c].enable
            && lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.rfConfs[c].tx_enable) {
            rfChain = c;
            break;
        }
    }
    if (rfChain < 0)
        return ERR_CODE_PARAM_INVALID;

    int freqOffset = -1;
    for (auto &ifConf: lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.ifConfs) {
        if (ifConf.enable && ifConf.rf_chain == rfChain) {
            freqOffset = ifConf.freq_hz;
            break;
        }
    }
    if (freqOffset == -1)
        return ERR_CODE_PARAM_INVALID;

    bool payloadIsDownlink = isDownlink(localConfig.payload.c_str(), localConfig.payload.size());

    // uint32_t center frequency of TX
    pkt.freq_hz = (uint32_t) (
            (int32_t) lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.rfConfs[rfChain].freq_hz + freqOffset);
    // ----------- TODO
    pkt.freq_hz = 869525000;

    if ((pkt.freq_hz < lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.tx_freq_min[rfChain]) ||
        pkt.freq_hz > lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.tx_freq_max[rfChain]) {
        // unsupported frequency
        return ERR_CODE_PARAM_INVALID;
    }

    pkt.tx_mode = IMMEDIATE;                // immediately uint8_t select on what event/time the TX is triggered
    pkt.count_us = 0;                       // immediately uint32_t timestamp or delay in microseconds for TX trigger
    pkt.rf_chain = (uint8_t) rfChain;       // uint8_t through which RF chain will the packet be sent
    pkt.rf_power = lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.txLut[rfChain].lut[0].rf_power;   // int8_t TX power, in dBm
    pkt.modulation = MODULATION_LORA;       // uint8_t modulation to use for the packet
    pkt.freq_offset = 0;                    // frequency offset from Radio Tx frequency (CW mode)
    pkt.bandwidth = BW_125KHZ;              // uint8_t modulation bandwidth (LoRa only)
    pkt.datarate = DR_LORA_SF9;             // DR_LORA_SF12 DR_LORA_SF9;             // uint32_t TX datarate (baudrate for FSK, SF for LoRa)
    pkt.coderate = CRLORA_4_5;              // uint8_t error-correcting code of the packet (LoRa only)
    pkt.invert_pol = payloadIsDownlink;     // bool invert signal polarity, for orthogonal downlinks (LoRa only)
    pkt.f_dev = 0;                          // uint8_t frequency deviation, in kHz (FSK only)
    pkt.preamble = STD_LORA_PREAMBLE;       // uint16_t set the preamble length, 0 for default

    // Correct radio transmission power
    pkt.rf_power -= lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.antennaGain;

    // check minimum preamble size
    if (pkt.modulation == MODULATION_LORA) {
        if (pkt.preamble == 0)
            pkt.preamble = STD_LORA_PREAMBLE;
    } else {
        if (pkt.preamble == 0)
            pkt.preamble = STD_FSK_PREAMBLE;
    }
    // translate "soft" bandwidth index into hardware index
    switch (pkt.bandwidth) {
        case BANDWIDTH_INDEX_125KHZ:
            pkt.bandwidth = BW_125KHZ;
            break;
        case BANDWIDTH_INDEX_250KHZ:
            pkt.bandwidth = BW_250KHZ;
            break;
        case BANDWIDTH_INDEX_500KHZ:
            pkt.bandwidth = BW_500KHZ;
            break;
        default:
            pkt.bandwidth = BW_125KHZ;
    }

    pkt.no_crc = payloadIsDownlink;                     // bool if true, do not send a CRC in the packet
    pkt.no_header = false;                              // bool if true, enable implicit header mode (LoRa), fixed length (FSK)
    pkt.size = (uint16_t) localConfig.payload.size();   // uint16_t payload size in bytes
    memmove(pkt.payload, localConfig.payload.c_str(), localConfig.payload.size());
    int r = lgw_send(&pkt);
    if (r)
        return ERR_CODE_LORA_GATEWAY_SEND_FAILED;
    waitSent(pkt.rf_chain, 3);
    return CODE_OK;
}

// max number of packets per fetch/send cycle
#define NB_PKT_MAX         255
// ms waited when a fetch return no packets
#define UPSTREAM_FETCH_DELAY_MS     10

static int listen4addr(
    struct lgw_pkt_rx_s &retVal,
    const LocalSenderConfiguration &config,
    const DEVADDR &devAddr,
    UsbLoRaWANGateway &gateway
) {
    struct lgw_pkt_rx_s rxpkt[NB_PKT_MAX]; // array containing inbound packets + metadata
    while (!config.stopRequest) {
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

        // search for address and return RX packets if found
        for (int i = 0; i < nb_pkt; ++i) {
            auto p = &rxpkt[i];
            DEVADDR a;
            if (getDEVADDR(a, p->payload, p->size) != 0)
                continue;
            if (a == devAddr) {
                memmove(&retVal, p, sizeof(struct lgw_pkt_rx_s));
                return CODE_OK;
            }
        }
    }
    return ERR_CODE_HANGUP_DETECTED;
}

static void run() {
    if (localConfig.verbosity > 1)
        std::cout << "Region " << localConfig.regionGWIdx << ' ' << lorawanGatewaySettings[localConfig.regionGWIdx].name
                  << '\n';
    setSignalHandler();
    // initialize all gateways. It seems like only one gateway is possible to serve.
    for (int deviceIndex = 0; deviceIndex < localConfig.devicePaths.size(); deviceIndex++) {
        strncpy(lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.boardConf.com_path,
                localConfig.devicePaths[deviceIndex].c_str(),
                sizeof(lorawanGatewaySettings[localConfig.regionGWIdx].sx130x.boardConf.com_path));
        localConfig.gateway.emplace_back();
        auto &l = localConfig.gateway.back();
        // if fail, delete gateway from the list
        if (l.init(&lorawanGatewaySettings[localConfig.regionGWIdx]))
            localConfig.gateway.pop_back();
        else
            if (lgw_start())
                localConfig.gateway.pop_back();
            else
                if (lgw_get_eui(&l.eui)) // get the concentrator EUI
                    localConfig.gateway.pop_back();
    }
    if (localConfig.gateway.empty()) {
        std::cerr << _("Invalid device: no any gateway found") << std::endl;
        return;
    }
    if (localConfig.classC) {
        for (auto &l: localConfig.gateway) {
            sendClassC(l);
        }
    } else {
        // get device address
        DEVADDR a;
        int r = getDEVADDR(a, (const uint8_t *) localConfig.payload.c_str(), localConfig.payload.size());
        if (r) {
            std::cerr << _("Invalid packet: no address found") << std::endl;
            return;
        }

        // listen first gateway
        auto &l = localConfig.gateway.front();

        std::cout << "Listen for uplink from device address " << DEVADDR2string(a)
                  << " from  gateway " << gatewayId2str(l.eui) << std::endl;
        struct lgw_pkt_rx_s rx{};
        r = listen4addr(rx, localConfig, a, l);
        if (r) {
            std::cerr << _("No uplink received: can not get RX1 or RX2 window time, exit") << std::endl;
            return;
        }
        // send in window
        if (localConfig.rx1) {
            r = sendClassARx1(rx);
            if (r)
                std::cerr << _("Error send in RX1 window") << std::endl;
        }
        if (localConfig.rx2) {
            r = sendClassARx2(rx);
            if (r)
                std::cerr << _("Error send in RX2 window") << std::endl;
        }
    }

    sleep(1);
    // stop all gateways
    for (auto &l: localConfig.gateway) {
        lgw_stop();
    }
}

int main(
	int argc,
	char *argv[]
)
{
    int r = parseCmd(&localConfig, argc, argv);
    if (r)
        return r;
    run();
    return CODE_OK;
}
