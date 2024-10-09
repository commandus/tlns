/**
 * LoRaWAN RAK2287 gateway backend
 */
#include <iostream>
#include <cstring>
#include <csignal>

#define DEF_UNIX_SOCKET_FILE_NAME "/tmp/usb.socket"

#if defined(_MSC_VER) || defined(__MINGW32__)
#else
#include <execinfo.h>
#include <algorithm>

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
#include "lorawan/task/task-unix-control-socket.h"
#include "lorawan/proto/gw/basic-udp.h"
#include "lorawan/storage/client/plugin-client.h"
#include "lorawan/bridge/plugin-bridge.h"
#include "lorawan/bridge/stdout-bridge.h"
#include "lorawan/storage/client/device-best-gateway-direct-client.h"
#include "lorawan/storage/service/device-best-gateway-mem.h"

// i18n
// #include <libintl.h>
// #define _(String) gettext (String)
#define _(String) (String)

size_t findRegionIndex(
    const std::string &namePrefix
)
{
    std::string upperPrefix(namePrefix);
    std::transform(upperPrefix.begin(), upperPrefix.end(), upperPrefix.begin(), ::toupper);
    for (size_t i = 0; i < sizeof(lorawanGatewaySettings) / sizeof(GatewaySettings); i++) {
        std::string upperName(lorawanGatewaySettings[i].name);
        std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
        if (upperName.find(upperPrefix) != std::string::npos)
            return i;
    }
    return 0;
}

const std::string programName = _("lorawan-gateway");
static TaskSocket *taskUSBSocket = nullptr;

class LocalGatewayConfiguration {
public:
    std::string devicePath;
    std::string identityFileName;
    std::string gatewayFileName;
    std::string pluginFilePath;
    std::vector<std::string> bridgePluginFiles;

    size_t regionIdx;
    bool enableSend;
    bool enableBeacon;
    bool daemonize;
    int verbosity;
    std::string pidfile;
    std::string unixSocketFileName;
    LocalGatewayConfiguration()
        : regionIdx(0), enableSend(false), enableBeacon(false), daemonize(false), verbosity(0) {

    }
};

GatewaySettings* getGatewayConfig(LocalGatewayConfiguration *config) {
    // set COM port device path, just in case
    strncpy(lorawanGatewaySettings[config->regionIdx].sx130x.boardConf.com_path, config->devicePath.c_str(),
            sizeof(lorawanGatewaySettings[config->regionIdx].sx130x.boardConf.com_path));
    return &lorawanGatewaySettings[config->regionIdx];
}

class StdErrLog: public Log {
private:
    MessageTaskDispatcher *dispatcher;
public:
    explicit StdErrLog(
        MessageTaskDispatcher *aDispatcher
    )
        : dispatcher(aDispatcher)
    {

    }

    void log(
        int level,
        const std::string &msg
    ) override
    {
        std::cerr << msg << std::endl;
        if (level <= LOG_ALERT) {
            if (dispatcher) {
                dispatcher->stop();
            }
        }
    }
};

static LocalGatewayConfiguration localConfig;
static MessageTaskDispatcher dispatcher;
static StdErrLog errLog(&dispatcher);

static void stop()
{
    dispatcher.stop();
}

static void done()
{
}

/**
 * Parse command line
 * Return 0- success
 *        ERR_CODE_PARAM_INVALID- show help and exit, or command syntax error
 **/
int parseCmd(
    LocalGatewayConfiguration *config,
    int argc,
    char *argv[]
)
{
    // device path
    struct arg_str *a_device_path = arg_str1(nullptr, nullptr, _("<device-name>"), _("USB gateway device e.g. /dev/ttyACM0"));
    struct arg_str *a_region_name = arg_str1("c", "region", _("<region-name>"), _("Region name, e.g. \"EU433\" or \"US\""));

    struct arg_str *a_identity_plugin_file = arg_str0("p", "plugin", _("<identity-plugin-file-name>"), _("Default none"));
    struct arg_str *a_identity_file_name = arg_str0("i", "id", _("<id-file-name>"), _("Device identities JSON file name"));
    struct arg_str *a_gateway_file_name = arg_str0("g", "gw", _("<gw-file-name>"), _("Gateways JSON file name"));

    struct arg_str *a_bridge_plugin = arg_strn("o", "output", _("<directory>"), 0, 64, _("Output plugins directory"));

    struct arg_lit *a_enable_send = arg_lit0("s", "allow-send", _("Allow send"));
    struct arg_lit *a_enable_beacon = arg_lit0("b", "allow-beacon", _("Allow send beacon"));
    struct arg_lit *a_daemonize = arg_lit0("d", "daemonize", _("Run as daemon"));
    struct arg_str *a_unix_socket_file = arg_str0("u", "socket-name", _("<file>"), _("UNIX socket file name. Default /tmp/usb.socket"));
    struct arg_str *a_pidfile = arg_str0("p", "pidfile", _("<file>"), _("Check whether a process has created the file pidfile"));
    struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 7, _("Verbosity level 1- alert, 2-critical error, 3- error, 4- warning, 5- siginicant info, 6- info, 7- debug"));
    struct arg_lit *a_help = arg_lit0("?", "help", _("Show this help"));
    struct arg_end *a_end = arg_end(20);

    void *argtable[] = {
        a_device_path, a_region_name, a_identity_plugin_file, a_identity_file_name, a_gateway_file_name,
        a_bridge_plugin,
        a_enable_send, a_enable_beacon,
        a_daemonize, a_unix_socket_file,
        a_pidfile, a_verbosity, a_help, a_end
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

    // try load shared library
    if (a_identity_plugin_file->count > 0) {
        if (!file::fileExists(*a_identity_plugin_file->sval)) {
            nErrors++;
            std::cerr
                << ERR_MESSAGE << ERR_CODE_LOAD_PLUGINS_FAILED << ": "
                << ERR_LOAD_PLUGINS_FAILED
                << *a_identity_plugin_file->sval
                << std::endl;
        } else
            config->pluginFilePath = *a_identity_plugin_file->sval;
    }

    if (a_identity_file_name->count)
        config->identityFileName = *a_identity_file_name->sval;
    else
        config->identityFileName = "";
    if (a_gateway_file_name->count)
        config->gatewayFileName = *a_gateway_file_name->sval;
    else
        config->gatewayFileName = "";

    for (int i = 0; i < a_bridge_plugin->count; i++)
        config->bridgePluginFiles.emplace_back(a_bridge_plugin->sval[i]);

    if (a_region_name->count)
        config->regionIdx = findRegionIndex(*a_region_name->sval);
    else
        config->regionIdx = 0;

    config->enableSend = (a_enable_send->count > 0);
    config->enableBeacon = (a_enable_beacon->count > 0);

    config->daemonize = (a_daemonize->count > 0);
    if (a_pidfile->count)
        config->pidfile = *a_pidfile->sval;
    else
        config->pidfile = "";

    if (a_unix_socket_file->count)
        config->unixSocketFileName = *a_unix_socket_file->sval;
    else
        config->unixSocketFileName = DEF_UNIX_SOCKET_FILE_NAME;

    config->verbosity = a_verbosity->count;

    // special case: '--help' takes precedence over error reporting
    if ((a_help->count) || nErrors) {
        if (nErrors)
            arg_print_errors(stderr, a_end, programName.c_str());
        std::cerr << _("Usage: ") << programName << std::endl;
        arg_print_syntax(stderr, argtable, "\n");
        std::cerr << MSG_PROG_NAME_GATEWAY_USB << std::endl;
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
            run();
            break;
        default:
            break;
    }
}

void setSignalHandler()
{
#ifndef _MSC_VER
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
    PluginClient identityClient(localConfig.pluginFilePath);
    if (!identityClient.svcIdentity || !identityClient.svcGateway) {
        std::cerr << ERR_MESSAGE << ERR_CODE_LOAD_PLUGINS_FAILED << ": " << ERR_LOAD_PLUGINS_FAILED << std::endl;
        return;
    }

    PluginBridges pluginBridges;
    pluginBridges.add(localConfig.bridgePluginFiles);
    for (auto &b : pluginBridges.bridges) {
        dispatcher.addAppBridge(b.bridge);
    }
    if (dispatcher.appBridges.empty()) {
        // add simple output bridge
        dispatcher.addAppBridge(new StdoutBridge);
    }

    identityClient.svcIdentity->init(localConfig.identityFileName, nullptr);
    identityClient.svcGateway->init(localConfig.gatewayFileName, nullptr);
    if (localConfig.verbosity > 1) {
        std::cout
            << MSG_IDENTITIES << identityClient.svcIdentity->size() << '\n'
            << MSG_GATEWAYS << identityClient.svcGateway->size() << std::endl;
    }

    dispatcher.setIdentityClient(&identityClient);
    dispatcher.onReceiveRawData = [] (
        MessageTaskDispatcher* aDispatcher,
        const char *buffer,
        size_t bufferSize,
        TASK_TIME receivedTime
    )
    {

        // Print out received row packet
        if (localConfig.verbosity)
            std::cout << hexString(buffer, bufferSize) << std::endl;

        // filter messages: set false to block packet, true to start processing
        return true;
    };

    dispatcher.onPushData = [] (
        MessageTaskDispatcher* dispatcher,
        MessageQueueItem *item
    ) {
        /*
        if (item)
            std::cout << item->toJsonString() << std::endl;
        */
    };

    dispatcher.onPullResp = [] (
        MessageTaskDispatcher* dispatcher,
        GwPullResp &item
    ) {
        std::cout
            << "{\"metadata\": "
            << SEMTECH_PROTOCOL_METADATA_TX2string(item.txMetadata)
            << ", \"gwId\": " << gatewayId2str(item.gwId.u)
            << ", \"txData\": " << item.txData.toString()
            << "}" << std::endl;
    };

    dispatcher.onTxPkAck = [] (
        MessageTaskDispatcher* dispatcher,
        ERR_CODE_TX code
    ) {
        std::cout
            << R"({"txPkAck": ")"
            << ERR_CODE_TX2string(code)
            << "\"}" << std::endl;
    };

    dispatcher.onDestroy = [] (
        MessageTaskDispatcher* dispatcher
    ) {
    };

    dispatcher.onError = [] (
        MessageTaskDispatcher* dispatcher,
        int level,
        const std::string &module,
        int code,
        const std::string &message
    ) {
        std::cerr << ERR_MESSAGE << code << ": " << message << " ("<< module<< ")" << std::endl;
    };

    DeviceBestGatewayServiceMem m(0, nullptr);
    DeviceBestGatewayDirectClient deviceBestGatewayDirectClient(&m);
    dispatcher.setDeviceBestGatewayClient(&deviceBestGatewayDirectClient);

    GatewaySettings* settings = getGatewayConfig(&localConfig);

    taskUSBSocket = new TaskUsbGatewayUnixSocket(&dispatcher, localConfig.unixSocketFileName, settings, &errLog,
        localConfig.enableSend, localConfig.enableBeacon, localConfig.verbosity);
    dispatcher.sockets.push_back(taskUSBSocket);

    // control socket
    TaskSocket *taskControlSocket = new TaskUnixControlSocket(localConfig.unixSocketFileName);
    dispatcher.sockets.push_back(taskControlSocket);
    dispatcher.setControlSocket(taskControlSocket);

    ProtoGwParser *parser = new GatewayBasicUdpProtocol(&dispatcher);
    dispatcher.addParser(parser);
    if (!localConfig.daemonize)
        setSignalHandler();
    // run() in main thread
    dispatcher.run();
}

int main(
	int argc,
	char *argv[]
)
{
    int r = parseCmd(&localConfig, argc, argv);
    if (r)
        return r;
    if (localConfig.daemonize)	{
        Daemonize daemonize(programName, getCurrentDir(), run, stop, done, 0, localConfig.pidfile);
    } else {
        run();
        stop();
        done();
    }
    return CODE_OK;
}
