#include <string>
#include <iostream>
#include <sstream>
#include <unistd.h>

/**
 * LoRaWAN node device simulator
 */
#ifdef _MSC_VER
#include <WinSock2.h>
#endif

#include "argtable3/argtable3.h"

#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-msg.h"
#include "lorawan/helper/file-helper.h"
#include "lorawan/storage/client/plugin-client.h"
#include "lorawan/storage/service/identity-service-json.h"

// i18n
#include <libintl.h>
#include <thread>
#include <lorawan/helper/ip-address.h>
#include <lorawan/proto/gw/json-wired-client.h>
#define _(String) gettext (String)
/// #define _(String) (String)

const char *programName = "wire-send";

// global parameters keep command line arguments
class WireSendParams {
public:
    int verbose;
    uint64_t gwId;
    std::string networkServerAddress;
    uint16_t networkServerPort;
    DEVADDR deviceAddress;
    std::string fopts;
    std::string payload;
    std::string identityFileName;
    std::string pluginFilePath;
    int32_t retCode;

    WireSendParams()
        : verbose(0), gwId(0), networkServerPort(4242), retCode(0)
    {

    }

    std::string toString() const {
        std::stringstream ss;
        ss << _("Network service") << '\t' << networkServerAddress  << ":" << networkServerPort << "\n"
            << _("Gateway") << '\t' << gatewayId2str(gwId) << "\n"
            << _("Device address") << '\t' << DEVADDR2string(deviceAddress) << "\n"
            << _("FOpts") << '\t' << hexString(fopts) << "\n"
            << _("Payload") << '\t' << hexString(payload) << "\n"
            << _("Identity file") << '\t' << identityFileName << "\n"
        ;
        return ss.str();
    }

};

static WireSendParams params;

static void run() {
    PluginClient identityClient(params.pluginFilePath);
    if (!identityClient.svcIdentity) {
        identityClient.svcIdentity = new JsonIdentityService();
    }
    identityClient.svcIdentity->init(params.identityFileName, nullptr);
    if (params.verbose > 1) {
        std::cout
            << MSG_IDENTITIES << identityClient.svcIdentity->size() << '\n'
            << params.toString() << std::endl;
    }
    DEVICEID did;
    DEVADDR devAddr(params.deviceAddress);
    int r = identityClient.svcIdentity->get(did, devAddr);
    if (r) {
        std::cerr << ERR_DEVICE_ADDRESS_NOTFOUND << std::endl;
        return;
    }

    JsonWiredClient jsonWiredClient(&identityClient,
        params.gwId, params.networkServerAddress, params.networkServerPort, params.deviceAddress);

    std::thread thread(std::bind(&JsonWiredClient::run, &jsonWiredClient));
    // wait thread is running
    while (jsonWiredClient.status != CODE_OK) {
        sleep(1);
    }

    std::cout << _("Enter 'q' to stop") << std::endl;
    while (jsonWiredClient.status != ERR_CODE_STOPPED) {
        std::string l;
        getline(std::cin, l);
        if (l == "q") {
            jsonWiredClient.stop();
            break;
        }
    }
    thread.join();
}

int main(int argc, char **argv) {
    struct arg_str *a_gateway_id = arg_str1("g", "gateway", _("<hex-number>"), _("Gateway identifer"));
    struct arg_str *a_network_server_address_n_port = arg_str1("s", "service", _("<IP address>:<port number>"), _("Network service address and port"));
    struct arg_str *a_device_address = arg_str1("a", "address", _("<hex-number>"), _("Device address"));
    struct arg_str *a_fopts = arg_str0("o", "fopts", _("<hex-sequence>"), _("FOpts (up to 16 bytes)"));
    struct arg_str *a_payload = arg_str0(nullptr, nullptr, _("<hex-sequence>"), _("payload (up to 255 bytes)"));
    struct arg_str *a_identity_plugin_file = arg_str0("p", "plugin", _("<identity-plugin-file-name>"), _("Default JSON file"));
    struct arg_str *a_identity_file_name = arg_str1("i", "identity", _("<file>"), _("Identities JSON file name"));

    struct arg_lit *a_verbose = arg_litn("v", "verbose", 0, 2, _("-v verbose -vv debug"));
    struct arg_lit *a_help = arg_lit0("h", "help", _("Show this help"));
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = {
	    a_gateway_id, a_network_server_address_n_port, a_device_address, a_fopts, a_payload,
	    a_identity_plugin_file, a_identity_file_name,
	    a_verbose, a_help, a_end
	};

	// verify the argtable[] entries were allocated successfully
	if (arg_nullcheck(argtable) != 0) {
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}
	// Parse the command line as defined by argtable[]
	int errorCount = arg_parse(argc, argv, argtable);

    params.verbose = a_verbose->count;

    if (!errorCount) {
        params.gwId = string2gatewayId(*a_gateway_id->sval);
        if (!splitAddress(params.networkServerAddress, params.networkServerPort, *a_network_server_address_n_port->sval))
            errorCount++;
        string2DEVADDR(params.deviceAddress, *a_device_address->sval);
        if (a_identity_plugin_file->count > 0) {
            if (!file::fileExists(*a_identity_plugin_file->sval)) {
                errorCount++;
                std::cerr
                    << ERR_MESSAGE << ERR_CODE_LOAD_PLUGINS_FAILED << ": "
                    << ERR_LOAD_PLUGINS_FAILED
                    << *a_identity_plugin_file->sval
                    << std::endl;
            } else
                params.pluginFilePath = *a_identity_plugin_file->sval;
        }
        params.identityFileName = *a_identity_file_name->sval;
    }
    if (a_fopts->count)
        params.fopts = hex2string(*a_fopts->sval);
    if (a_payload->count)
        params.payload = hex2string(*a_payload->sval);

    // special case: '--help' takes precedence over error reporting
	if ((a_help->count) || errorCount) {
		if (errorCount)
			arg_print_errors(stderr, a_end, programName);
		std::cerr << _("Usage: ") << programName << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << _("Send via wires from the device") << std::endl;
		arg_print_glossary(stderr, argtable, "  %-27s %s\n");
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
