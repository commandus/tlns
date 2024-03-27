/**
 * LoRaWAN RAK2287 gateway backend
 */
#include <iostream>
#include <cstring>
#include <csignal>

#ifdef _MSC_VER
#else
#include <execinfo.h>
#endif

#include <fcntl.h>

#include "argtable3/argtable3.h"

#include "libloragw-helper.h"

#include "daemonize.h"
#include "lorawan/lorawan-error.h"

// generated gateway regional settings source code
#include "gateway_usb_conf.cpp"
#include "lorawan/lorawan-msg.h"
#include "lorawan/helper/file-helper.h"

#include "subst-call-c.h"

class PosixLibLoragwOpenClose : public LibLoragwOpenClose {
private:
        std::string devicePath;
public:
    
    explicit PosixLibLoragwOpenClose(const std::string &aDevicePath) : devicePath(aDevicePath) {};

    int openDevice(const char *fileName, int mode) override
    {
        return open(devicePath.c_str(), mode);
    };

    int closeDevice(int fd) override
    {
        return close(fd);
    };
};

static std::string getRegionNames()
{
    std::stringstream ss;
    for (size_t i = 0; i < sizeof(lorawanGatewaySettings) / sizeof(GatewaySettings); i++) {
        ss << "\"" << lorawanGatewaySettings[i].name << "\" ";
    }
    return ss.str();
}

size_t findRegionIndex(
    const std::string &namePrefix
)
{
    for (size_t i = 0; i < sizeof(lorawanGatewaySettings) / sizeof(GatewaySettings); i++) {
        if (lorawanGatewaySettings[i].name.find(namePrefix) != std::string::npos) {
            return i;
        }
    }
    return 0;
}

const std::string programName = "lorawan-gateway";

class LocalGatewayConfiguration {
public:
    std::string devicePath;
    std::string identityFileName;
    size_t regionIdx;
    bool enableSend;
    bool enableBeacon;
    bool daemonize;
    int verbosity;

};

GatewaySettings* getGatewayConfig(LocalGatewayConfiguration *config) {
    // set COM port device path, just in case
    strncpy(lorawanGatewaySettings[config->regionIdx].sx130x.boardConf.com_path, config->devicePath.c_str(),
            sizeof(lorawanGatewaySettings[config->regionIdx].sx130x.boardConf.com_path));
    return &lorawanGatewaySettings[config->regionIdx];
}

static LocalGatewayConfiguration localConfig;

static void stop()
{
}

static LibLoragwHelper libLoragwHelper;

static void done()
{
    if (libLoragwHelper.onOpenClose) {
        delete libLoragwHelper.onOpenClose;
        libLoragwHelper.onOpenClose = nullptr;
    }
}

class StdErrLog: public Log {
public:
    std::ostream& strm(int level) override {
        return std::cerr;
    }
};

/**
 * Parse command line
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int parseCmd(
    LocalGatewayConfiguration *config,
    int argc,
    char *argv[]
)
{
    // device path
    struct arg_str *a_device_path = arg_str1(nullptr, nullptr, "<device-name>", "USB gateway device e.g. /dev/ttyACM0");
    struct arg_str *a_region_name = arg_str1("c", "region", "<region-name>", "Region name, e.g. \"EU433\" or \"US\"");
    struct arg_str *a_identity_file_name = arg_str0("i", "id", "<id-file-name>", "Device identities JSON file name");
    struct arg_lit *a_enable_send = arg_lit0("s", "allow-send", "Allow send");
    struct arg_lit *a_enable_beacon = arg_lit0("b", "allow-beacon", "Allow send beacon");
    struct arg_lit *a_daemonize = arg_lit0("d", "daemonize", "Run as daemon");
    struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 7, "Verbosity level 1- alert, 2-critical error, 3- error, 4- warning, 5- siginicant info, 6- info, 7- debug");
    struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
    struct arg_end *a_end = arg_end(20);

    void *argtable[] = {
        a_device_path, a_region_name, a_identity_file_name,
        a_enable_send, a_enable_beacon,
        a_daemonize, a_verbosity, a_help, a_end
    };

    // verify the argtable[] entries were allocated successfully
    if (arg_nullcheck(argtable) != 0) {
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return ERR_CODE_PARAM_INVALID;
    }
    // Parse the command line as defined by argtable[]
    int nErrors = arg_parse(argc, argv, argtable);

    if (a_device_path->count)
        config->devicePath = std::string(*a_device_path->sval);
    else
        config->devicePath = "";
    if (a_identity_file_name->count)
        config->identityFileName = *a_identity_file_name->sval;
    else
        config->identityFileName = "";

    if (a_region_name->count)
        config->regionIdx = findRegionIndex(*a_region_name->sval);
    else
        config->regionIdx = 0;

    config->enableSend = (a_enable_send->count > 0);
    config->enableBeacon = (a_enable_beacon->count > 0);

    config->daemonize = (a_daemonize->count > 0);
    config->verbosity = a_verbosity->count;

    // special case: '--help' takes precedence over error reporting
    if ((a_help->count) || nErrors) {
        if (nErrors)
            arg_print_errors(stderr, a_end, programName.c_str());
        std::cerr << "Usage: " << programName << std::endl;
        arg_print_syntax(stderr, argtable, "\n");
        std::cerr << MSG_PROG_NAME_GATEWAY_USB << std::endl;
        arg_print_glossary(stderr, argtable, "  %-25s %s\n");
        std::cerr << "  region name: "
            << getRegionNames() << std::endl;
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
    size_t size = backtrace(t, TRACE_BUFFER_SIZE);
    backtrace_symbols_fd(t, size, STDERR_FILENO);
#endif
}

static StdErrLog errLog;

static void init();
static void run();

void signalHandler(int signal)
{
    // lastSysSignal = signal;
    switch (signal)
    {
        case SIGINT:
            std::cerr << MSG_INTERRUPTED << std::endl;
            stop();
            done();
            break;
        case SIGSEGV:
            std::cerr << ERR_SEGMENTATION_FAULT << std::endl;
            printTrace();
            exit(ERR_CODE_SEGMENTATION_FAULT);
        case SIGABRT:
            std::cerr << ERR_ABRT << std::endl;
            printTrace();
            exit(ERR_CODE_ABRT);
#ifndef _MSC_VER
        case SIGHUP:
            std::cerr << ERR_HANGUP_DETECTED << std::endl;
            break;
        case SIGUSR2:	// 12
            std::cerr << MSG_SIG_FLUSH_FILES << std::endl;
            // flushFiles();
            break;
#endif
        case 42:	// restart
            std::cerr << MSG_RESTART_REQUEST << std::endl;
            stop();
            done();
            init();
            run();
            break;
        default:
            break;
    }
}

void setSignalHandler()
{
#ifndef _MSC_VER
    struct sigaction action;
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
    if (!localConfig.daemonize)
        setSignalHandler();
}

static void init()
{
    libLoragwHelper.bind(&errLog, new PosixLibLoragwOpenClose(localConfig.devicePath));
    if (!libLoragwHelper.onOpenClose)
        return;

    if (localConfig.identityFileName.empty()) {
        // std::cerr << ERR_WARNING << ERR_CODE_INIT_IDENTITY << ": " << ERR_INIT_IDENTITY << std::endl;
        if (localConfig.verbosity > 0)
            std::cerr << MSG_NO_IDENTITIES << std::endl;
    } else {
    }

    libLoragwHelper.bind(&errLog, nullptr);
}

int main(
	int argc,
	char *argv[])
{
    if (parseCmd(&localConfig, argc, argv) != 0) {
        // std::cerr << ERR_MESSAGE << ERR_CODE_COMMAND_LINE << ": " << ERR_COMMAND_LINE << std::endl;
        exit(ERR_CODE_COMMAND_LINE);
    }
    init();

    if (localConfig.daemonize)	{
        std::string progpath = getCurrentDir();
        Daemonize daemonize(programName, progpath, run, stop, done);
    } else {
        run();
        done();
    }
    return 0;
}
