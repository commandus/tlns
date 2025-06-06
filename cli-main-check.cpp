#include <string>
#include <iostream>
#include <sstream>

#include "argtable3/argtable3.h"

#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/task/message-queue.h"
#include "lorawan/task/message-task-dispatcher.h"
#include "lorawan/proto/gw/basic-udp.h"

#include "gen/regional-parameters-3.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <WinSock2.h>
#else
#include "lorawan/task/task-unix-socket.h"
#include "lorawan/task/task-unix-control-socket.h"
#endif

// i18n
#include <libintl.h>
#define _(String) gettext (String)
/// #define _(String) (String)

const char *programName = "tlns-check";
const char *FILE_NAME_UNIX_SOCKET = "/tmp/cli-main-check.socket";

static void printRegionNames(
    std::ostream &strm
)
{
    for (auto & regionalPlan : regionalParameterChannelPlanMem.storage.bands) {
        strm << "\"" << regionalPlan.get()->cn << "\" ";
    }
}

// global parameters keep command line arguments
class CheckParams {
public:
    int verbose;
    std::string regionName;
    int32_t retCode;
    const RegionalParameterChannelPlan *regionalParameterChannelPlan{};

    CheckParams()
        : verbose(0), retCode(0)
    {

    }

    std::string toString() const {
        std::stringstream ss;
        ss << _("Verbose: ") << (verbose ? "true":"false") << "\n";
        return ss.str();
    }

};

static CheckParams params;

bool onReceiveRawData(
    MessageTaskDispatcher* dispatcher,
    const char *buffer,
    size_t bufferSize,
    TASK_TIME receivedTime
)
{
    if (params.verbose)
        std::cout << hexString(buffer, bufferSize) << std::endl;
    // filter messages: set false to block packet, true to start processing
    return true;
}

static void run() {
    MessageTaskDispatcher dispatcher;
    ProtoGwParser *parser = new GatewayBasicUdpProtocol(&dispatcher);
    dispatcher.addParser(parser);

    dispatcher.onPushData = [] (
        MessageTaskDispatcher* dispatcher,
        MessageQueueItem *item
    ) {
        std::cout
            << "{\"metadata\": [";
        bool f = true;
        for (auto & it : item->metadata) {
            if (f)
                f = false;
            else
                std::cout << ", ";
            std::cout << "{\"gateway_id\": " << gatewayId2str(it.first)
                << ", \"sock_addr\": " << sockaddr2string(&it.second.addr)
                << ", \"metadata\": " << SEMTECH_PROTOCOL_METADATA_RX2string(it.second.rx) << "}";
        }
        std::cout
            << "],\n\"rfm\": "
            << item->radioPacket.toString()
            << "}" << std::endl;
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

    if (params.verbose) {
        std::cout << _("Region") << '\t' << params.regionalParameterChannelPlan->get()->cn << std::endl;
    }

    // set selected regional parameters e.g. frequency
    dispatcher.setRegionalParameterChannelPlan(params.regionalParameterChannelPlan);

    // dispatcher 'll destroy sockets in destructor
    // dispatcher.sockets.push_back(new TaskUDPSocket(INADDR_LOOPBACK, 4242));
    // dispatcher.setControlSocket(new TaskUDPControlSocket(INADDR_LOOPBACK, 4242));
#if defined(_MSC_VER) || defined(__MINGW32__)
#else
    dispatcher.sockets.push_back(new TaskUnixSocket(FILE_NAME_UNIX_SOCKET));
    dispatcher.setControlSocket(new TaskUnixControlSocket(FILE_NAME_UNIX_SOCKET));
#endif
    dispatcher.onReceiveRawData = onReceiveRawData;
    // dispatcher.setControlSocket(new TaskEventFDControlSocket());
    dispatcher.start();

    // TaskResponseThreaded response;
    // dispatcher.setResponse(&response);

    std::cout << _("Enter packet (hex string) or 'q' to stop") << std::endl;
    while (true) {
        std::string l;
        getline(std::cin, l);
        if (l == "q") {
            dispatcher.stop();
            break;
        }
        l = "024c7e0000006cc3743eed467b227278706b223a5b7b22746d7374223a313237353533303937322c226368616e223a362c2272666368223a312c2266726571223a3836382e3930303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a22534631324257313235222c22636f6472223a22342f35222c226c736e72223a2d392e352c2272737369223a2d3131352c2273697a65223a33372c2264617461223a2251444144525147416e5259436b4c72715672703677324a55547958744a4467315669464a354d44666b756e336f762f5653513d3d227d2c7b22746d7374223a313237353533303938302c226368616e223a342c2272666368223a302c2266726571223a3836342e3930303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a22534631324257313235222c22636f6472223a22342f35222c226c736e72223a31302e382c2272737369223a2d32372c2273697a65223a33372c2264617461223a2251444144525147416e5259436b4c72715672703677324a55547958744a4467315669464a354d44666b756e336f762f5653513d3d227d5d7d";
        dispatcher.send2uplink(hex2string(l));
        if (dispatcher.state == TASK_STOPPED)
            break;
    }
    // dispatcher.stop();
    delete(parser);
}

static void printRegionNames(
    std::ostream &strm,
    const RegionalParameterChannelPlanMem &lorawanGatewaySettings
)
{
    for (auto &lorawanGatewaySetting : lorawanGatewaySettings.storage.bands)
        strm << "\"" << lorawanGatewaySetting.get()->name << "\" ";
}

int main(int argc, char **argv) {
    struct arg_lit *a_verbose = arg_litn("v", "verbose", 0, 2, _("-v verbose -vv debug"));
    struct arg_str *a_region_name = arg_str1("c", "region", _("<region-name>"), _("Region name, e.g. \"EU433\" or \"US\""));
    struct arg_lit *a_help = arg_lit0("h", "help", _("Show this help"));
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = {
        a_region_name, a_verbose,
		a_help, a_end
	};

	// verify the argtable[] entries were allocated successfully
	if (arg_nullcheck(argtable) != 0) {
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}
	// Parse the command line as defined by argtable[]
	int errorCount = arg_parse(argc, argv, argtable);

    params.verbose = a_verbose->count;

    if (a_region_name->count)
        params.regionName = *a_region_name->sval;
    else
        params.regionName = "";

    auto region= regionalParameterChannelPlanMem.get(params.regionName);

    if (!region) {
        errorCount++;
        std::cerr << _("Region ") << params.regionName << " " << _("not found") << std::endl;
    }

    if (errorCount) {
        std::cerr << _("  region name: ");
        printRegionNames(std::cerr, regionalParameterChannelPlanMem);
        std::cerr << std::endl;
    }

    params.regionalParameterChannelPlan = region;

    // special case: '--help' takes precedence over error reporting
	if ((a_help->count) || errorCount) {
		if (errorCount)
			arg_print_errors(stderr, a_end, programName);
		std::cerr << _("Usage: ") << programName << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << _("tlns check utility") << std::endl;
		arg_print_glossary(stderr, argtable, "  %-27s %s\n");
        std::cerr << _("Regions: ");
        printRegionNames(std::cerr);
        std::cerr << std::endl;
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    if (params.verbose) {
        std::cerr << params.toString() << std::endl;
    }

#ifdef _MSC_VER
    WSADATA wsaData;
    int r = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (r)
        return r;
#endif
	run();
#ifdef _MSC_VER
    WSACleanup();
#endif
    return params.retCode;
}
