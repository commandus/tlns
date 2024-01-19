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
#include "task-response-threaded.h"

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

SOCKET addExampleControlSocket(
    MessageTaskDispatcher *dispatcher,
    in_addr_t addr,
    uint16_t port
)
{
    dispatcher->controlSocket = new TaskSocket(addr, port, [] (
            MessageTaskDispatcher *dispatcher,
            const char *buffer,
            size_t size
    ) {
        if (size == 1 && *buffer == 'q') {
            dispatcher->running = false;
            return -1;
        }
        // add a new "received" packet
        std::cerr << "** Received " << std::endl;
        return 0;
    });
    dispatcher->sockets.push_back(dispatcher->controlSocket);
    return dispatcher->controlSocket->sock;
}

static void run() {
    MessageQueue q;
    MessageTaskDispatcher dispatcher;
    dispatcher.setQueue(&q);
    addDumbControlSocket(&dispatcher, INADDR_LOOPBACK, 4242);
    addExampleControlSocket(&dispatcher, INADDR_LOOPBACK, 4244);
    dispatcher.start();

    // TaskResponseThreaded response;
    // dispatcher.setResponse(&response);

    std::cout << "Enter 'q' to stop" << std::endl;
    while (true) {
        std::string l;
        getline(std::cin, l);
        if (!dispatcher.running)
            break;
    }
    // dispatcher.stop();
}

int main(int argc, char **argv) {
    struct arg_lit *a_verbose = arg_litn("v", "verbose", 0, 2,"-v verbose -vv debug");
    struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
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
		std::cerr << "Usage: " << programName << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "tlns check utility" << std::endl;
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
