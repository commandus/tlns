/**
 * LoRaWAN RAK2287 radio listener
 */
#include <iostream>
#include <cstring>
#include <csignal>
#include <algorithm>


#if defined(_MSC_VER) || defined(__MINGW32__)
#else
#include <execinfo.h>
#include <algorithm>
#include <sys/un.h>
#endif

#include "argtable3/argtable3.h"

#include "daemonize.h"
#include "lorawan/lorawan-error.h"

// generated gateway regional settings source code
#include "gen/gateway-usb-conf.h"
#include "lorawan/lorawan-msg.h"
#include "lorawan/helper/file-helper.h"

#include "task-usb-socket.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/storage/client/plugin-client.h"
#include "gateway-settings-helper.h"
#include "usb-listener.h"

// i18n
// #include <libintl.h>
// #define _(String) gettext (String)
#define _(String) (String)

const std::string programName = _("listen-lora-usb");

class LocalListenerConfiguration {
public:
    std::vector<UsbListener> listeners;
    std::vector <std::string> devicePaths;
    size_t regionIdx;
    bool daemonize;
    int verbosity;
    std::string pidfile;
    LocalListenerConfiguration()
        : regionIdx(0), daemonize(false), verbosity(0)
    {
    }
};

GatewaySettings* getGatewayConfig(LocalListenerConfiguration *config, int deviceIndex) {
    // set COM port device path, just in case
    strncpy(lorawanGatewaySettings[config->regionIdx].sx130x.boardConf.com_path, config->devicePaths[deviceIndex].c_str(),
            sizeof(lorawanGatewaySettings[config->regionIdx].sx130x.boardConf.com_path));
    return &lorawanGatewaySettings[config->regionIdx];
}

static LocalListenerConfiguration localConfig;

static void stop()
{
    for (auto &l : localConfig.listeners)
        l.stop(false);
}

static void done()
{
    localConfig.listeners.clear();
#ifdef _MSC_VER
    WSACleanup();
#endif
}

/**
 * Parse command line
 * Return 0- success
 *        ERR_CODE_PARAM_INVALID- show help and exit, or command syntax error
 **/
int parseCmd(
    LocalListenerConfiguration *config,
    int argc,
    char *argv[]
)
{
    // device path
    struct arg_str *a_device_path = arg_strn(nullptr, nullptr, _("<device-name>"), 1, 100, _("USB gateway device e.g. /dev/ttyACM0"));
    struct arg_str *a_region_name = arg_str1("c", "region", _("<region-name>"), _("Region name, e.g. \"EU433\" or \"US\""));
    struct arg_lit *a_daemonize = arg_lit0("d", "daemonize", _("Run as daemon"));
    struct arg_str *a_pidfile = arg_str0("p", "pidfile", _("<file>"), _("Check whether a process has created the file pidfile"));
    struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 7, _("Verbosity level 1- alert, 2-critical error, 3- error, 4- warning, 5- siginicant info, 6- info, 7- debug"));
    struct arg_lit *a_help = arg_lit0("?", "help", _("Show this help"));
    struct arg_end *a_end = arg_end(20);

    void *argtable[] = {
    a_device_path, a_region_name, a_pidfile, a_verbosity, a_help, a_end
    };

    // verify the argtable[] entries were allocated successfully
    if (arg_nullcheck(argtable) != 0) {
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return ERR_CODE_PARAM_INVALID;
    }
    // Parse the command line as defined by argtable[]
    int nErrors = arg_parse(argc, argv, argtable);

    for (int i = 0; i < a_device_path->count; i++)
        config->devicePaths.push_back(a_device_path->sval[i]);

    if (a_region_name->count)
        config->regionIdx = findGatewayRegionIndex(lorawanGatewaySettings, *a_region_name->sval);
    else
        config->regionIdx = 0;

    config->daemonize = (a_daemonize->count > 0);
    if (a_pidfile->count)
        config->pidfile = *a_pidfile->sval;
    else
        config->pidfile = "";
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

static void run();

#ifndef _MSC_VER
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
    if (!localConfig.daemonize) {
        if (localConfig.verbosity > 1)
            std::cout << "Region " << localConfig.regionIdx << ' ' << lorawanGatewaySettings[localConfig.regionIdx].name << '\n';
        setSignalHandler();
    }
    for (int deviceIndex = 0; deviceIndex < localConfig.devicePaths.size(); deviceIndex++) {
        strncpy(lorawanGatewaySettings[localConfig.regionIdx].sx130x.boardConf.com_path, localConfig.devicePaths[deviceIndex].c_str(),
                    sizeof(lorawanGatewaySettings[localConfig.regionIdx].sx130x.boardConf.com_path));
        localConfig.listeners.push_back(UsbListener{});
        auto &l = localConfig.listeners.back();
        if (l.init(&lorawanGatewaySettings[localConfig.regionIdx]) == 0)
            if (l.start() == 0)
                continue;
        // if fail, delete
        localConfig.listeners.pop_back();
    }
    // wait all
    for (auto &l : localConfig.listeners) {
        l.wait();
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
#ifdef _MSC_VER
    WSADATA wsaData;
    r = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (r)
        return r;
#endif
    if (localConfig.daemonize)
        Daemonize daemonize(programName, getCurrentDir(), run, stop, done, 0, localConfig.pidfile);
    else
        run();
    return CODE_OK;
}
