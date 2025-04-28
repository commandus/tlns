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

// i18n
// #include <libintl.h>
// #define _(String) gettext (String)
#define _(String) (String)

const std::string programName = _("send-lora-usb");

#define STD_LORA_PREAMBLE           8
#define STD_FSK_PREAMBLE            5

class LocalSenderConfiguration {
public:
    std::vector <UsbLoRaWANGateway> gateway;
    std::vector <std::string> devicePaths;
    size_t regionIdx;
    int verbosity;
    std::string payload;
    LocalSenderConfiguration()
        : regionIdx(0), verbosity(0)
    {
    }
};

static LocalSenderConfiguration localConfig;

static void stop()
{
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
    struct arg_str *a_device_path = arg_strn(nullptr, nullptr, _("<device-name>"), 1, 100, _("USB gateway device e.g. /dev/ttyACM0"));
    struct arg_str *a_region_name = arg_str1("c", "region", _("<region-name>"), _("Region name, e.g. \"EU433\" or \"US\""));
    struct arg_str *a_payload = arg_str1("p", "payload", _("<hex-string>"), _("Radio packet, hex string, e.g. 60e26a7e00000000026b69636b6173732d776f7a6e69616b5a54167c"));
    struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 7, _("Verbosity level 1- alert, 2-critical error, 3- error, 4- warning, 5- siginicant info, 6- info, 7- debug"));
    struct arg_lit *a_help = arg_lit0("?", "help", _("Show this help"));
    struct arg_end *a_end = arg_end(20);

    void *argtable[] = {
    a_device_path, a_region_name, a_payload, a_verbosity, a_help, a_end
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

    if (a_region_name->count)
        config->regionIdx = findGatewayRegionIndex(lorawanGatewaySettings, *a_region_name->sval);
    else
        config->regionIdx = 0;

    if (a_payload->count)
        config->payload = hex2string(a_payload->sval[0]);

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

static void run()
{
    if (localConfig.verbosity > 1)
        std::cout << "Region " << localConfig.regionIdx << ' ' << lorawanGatewaySettings[localConfig.regionIdx].name << '\n';
    setSignalHandler();
    // initialize
    for (int deviceIndex = 0; deviceIndex < localConfig.devicePaths.size(); deviceIndex++) {
        strncpy(lorawanGatewaySettings[localConfig.regionIdx].sx130x.boardConf.com_path, localConfig.devicePaths[deviceIndex].c_str(),
                    sizeof(lorawanGatewaySettings[localConfig.regionIdx].sx130x.boardConf.com_path));
        localConfig.gateway.emplace_back();
        auto &l = localConfig.gateway.back();
        if (l.init(&lorawanGatewaySettings[localConfig.regionIdx]) == 0)
            continue;
        // if fail, delete
        localConfig.gateway.pop_back();
    }
    // send over all gateways
    for (auto &l : localConfig.gateway) {
        int r = lgw_start();
        if (r)
            continue;
        // get the concentrator EUI
        r = lgw_get_eui(&l.eui);
        if (r)
            continue;
        struct lgw_pkt_tx_s pkt {};
        int rfChain = -1;
        for (int c = 0; c < LGW_RF_CHAIN_NB; c++) {
            if (lorawanGatewaySettings[localConfig.regionIdx].sx130x.rfConfs[c].enable) {
                rfChain = c;
                break;
            }
        }
        if (rfChain < 0)
            continue;

        int freqOffset = -1;
        for (auto & ifConf : lorawanGatewaySettings[localConfig.regionIdx].sx130x.ifConfs) {
            if (ifConf.enable && ifConf.rf_chain == rfChain) {
                freqOffset = ifConf.freq_hz;
                break;
            }
        }
        if (freqOffset == -1)
            continue;

        bool payloadIsDownlink = isDownlink(localConfig.payload.c_str(), localConfig.payload.size());

        // uint32_t center frequency of TX
        pkt.freq_hz = (uint32_t) ((int32_t) lorawanGatewaySettings[localConfig.regionIdx].sx130x.rfConfs[rfChain].freq_hz + freqOffset);
        pkt.tx_mode = IMMEDIATE;                // immediately uint8_t select on what event/time the TX is triggered
        pkt.count_us = 0;                       // immediately uint32_t timestamp or delay in microseconds for TX trigger
        pkt.rf_chain = (uint8_t ) rfChain;      // uint8_t through which RF chain will the packet be sent
        pkt.rf_power = lorawanGatewaySettings[localConfig.regionIdx].sx130x.txLut[rfChain].lut[0].rf_power;   // int8_t TX power, in dBm
        pkt.modulation = MODULATION_LORA;       // uint8_t modulation to use for the packet
        pkt.freq_offset = 0;                    // frequency offset from Radio Tx frequency (CW mode)
        pkt.bandwidth = BANDWIDTH_INDEX_125KHZ; // uint8_t modulation bandwidth (LoRa only)
        pkt.datarate = DRLORA_SF5;              // uint32_t TX datarate (baudrate for FSK, SF for LoRa)
        pkt.coderate = CRLORA_4_5;              // uint8_t error-correcting code of the packet (LoRa only)
        pkt.invert_pol = payloadIsDownlink;     // bool invert signal polarity, for orthogonal downlinks (LoRa only)
        pkt.f_dev = 0;                          // uint8_t frequency deviation, in kHz (FSK only)
        pkt.preamble = STD_LORA_PREAMBLE;       // uint16_t set the preamble length, 0 for default

        // Validate is channel allowed
        if (!lorawanGatewaySettings[localConfig.regionIdx].sx130x.rfConfs[pkt.rf_chain].tx_enable)
            continue;

        // Correct radio transmission power
        pkt.rf_power -= lorawanGatewaySettings[localConfig.regionIdx].sx130x.antennaGain;

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
        pkt.no_header = true;                               // bool if true, enable implicit header mode (LoRa), fixed length (FSK)
        pkt.size = (uint16_t) localConfig.payload.size();   // uint16_t payload size in bytes
        memmove(pkt.payload, localConfig.payload.c_str(), localConfig.payload.size());
        r = lgw_send(&pkt);
        if (r)
            continue;
        bool sent = false;
        while (true) {
            uint8_t c;
            r = lgw_status(rfChain, TX_STATUS, &c);
            if (r)
                break;
            if (c == TX_EMITTING) {
                sent = true;
                continue;
            }
            if (sent && c == TX_FREE)
                break;
        }
        sleep(1);
        r = lgw_stop();
        if (r)
            continue;
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
