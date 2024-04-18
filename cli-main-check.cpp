#include <string>
#include <iostream>
#include <sstream>

#ifdef _MSC_VER
#include <WinSock2.h>
#endif

#include "argtable3/argtable3.h"

#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/task/message-queue.h"
#include "lorawan/task/message-task-dispatcher.h"
#include "lorawan/task/task-udp-socket.h"
#include "lorawan/task/task-udp-control-socket.h"

// i18n
// #include <libintl.h>
// #define _(String) gettext (String)
#define _(String) (String)

const char *programName = "tlns-check";

// global parameters
class CheckParams {
public:
    int verbose;
    int32_t retCode;

    CheckParams()
        : verbose(0), retCode(0)
    {

    }

    std::string toString() const {
        std::stringstream ss;
        ss << "Verbose: " << verbose << "\n";
        return ss.str();
    }
};

static CheckParams params;

static void run() {
    MessageTaskDispatcher dispatcher;
    dispatcher.onPushData = [] (
        MessageTaskDispatcher* dispatcher,
        MessageQueueItem *item
    ) {
        std::cout
            << "{\"metadata\": [";
        bool f = true;
        for (auto it(item->metadata.begin()); it != item->metadata.end(); it++) {
            if (f)
                f = false;
            else
                std::cout << ", ";
            std::cout << "{\"gateway_id\": " << gatewayId2str(it->first);
            std::cout << ", \"metadata\": " << SEMTECH_PROTOCOL_METADATA_RX2string(it->second) << "}";
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
            << "{\"txPkAck\": \""
            << ERR_CODE_TX2string(code)
            << "\"}" << std::endl;
    };
    TaskSocket *ss = new TaskUDPSocket(INADDR_LOOPBACK, 4242);
    dispatcher.sockets.push_back(ss);

    TaskSocket *cs = new TaskUDPControlSocket(INADDR_LOOPBACK, 4242);
    dispatcher.sockets.push_back(cs);
    dispatcher.setControlSocket(cs);
    dispatcher.start();

    // TaskResponseThreaded response;
    // dispatcher.setResponse(&response);

    std::cout << _("Enter packet (hex string) or  'q' to stop") << std::endl;
    while (true) {
        std::string l;
        getline(std::cin, l);
        if (l == "q") {
            dispatcher.stop();
            break;
        }
        l = "024c7e0000006cc3743eed467b227278706b223a5b7b22746d7374223a313237353533303937322c226368616e223a362c2272666368223a312c2266726571223a3836382e3930303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a22534631324257313235222c22636f6472223a22342f35222c226c736e72223a2d392e352c2272737369223a2d3131352c2273697a65223a33372c2264617461223a2251444144525147416e5259436b4c72715672703677324a55547958744a4467315669464a354d44666b756e336f762f5653513d3d227d2c7b22746d7374223a313237353533303938302c226368616e223a342c2272666368223a302c2266726571223a3836342e3930303030302c2273746174223a312c226d6f6475223a224c4f5241222c2264617472223a22534631324257313235222c22636f6472223a22342f35222c226c736e72223a31302e382c2272737369223a2d32372c2273697a65223a33372c2264617461223a2251444144525147416e5259436b4c72715672703677324a55547958744a4467315669464a354d44666b756e336f762f5653513d3d227d5d7d";
        dispatcher.send(hex2string(l));
        if (!dispatcher.running)
            break;
    }
    // dispatcher.stop();
}

int main(int argc, char **argv) {
    struct arg_lit *a_verbose = arg_litn("v", "verbose", 0, 2, _("-v verbose -vv debug"));
    struct arg_lit *a_help = arg_lit0("h", "help", _("Show this help"));
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
        a_verbose,
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

    // special case: '--help' takes precedence over error reporting
	if ((a_help->count) || errorCount) {
		if (errorCount)
			arg_print_errors(stderr, a_end, programName);
		std::cerr << _("Usage: ") << programName << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << _("tlns check utility") << std::endl;
		arg_print_glossary(stderr, argtable, "  %-27s %s\n");
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
