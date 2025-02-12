#include <string>
#include <iostream>

#include <sstream>
#include <future>

#include "argtable3/argtable3.h"

#ifdef ENABLE_LIBUV
#include <uv.h>
#include "lorawan/storage/client/uv-client.h"
#else
#include "lorawan/storage/client/udp-client.h"
#endif
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-msg.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/lorawan-string.h"
#include "log.h"
#include "lorawan/storage/serialization/identity-serialization.h"
#include "lorawan/storage/serialization/identity-binary-serialization.h"
#include "lorawan/helper/ip-address.h"
#include "cli-helper.h"

#include "lorawan/storage/client/plugin-client.h"
#include "lorawan/storage/client/plugin-query-client.h"
#include "lorawan/storage/client/service-client.h"
#include "lorawan/helper/aes-helper.h"

const char *programName = "lorawan-identity-print";
#define DEF_PORT 4244

// i18n
// #include <libintl.h>
// #define _(String) gettext (String)
#define _(String) (String)

// global parameters
class CliPrintParams {
public:
    std::vector<std::string> payload;
    int svcOrPlugin; // 0- service, 1- shared library plugin, 2- static (embedded) plugin
    std::string address;
    uint16_t port;
    bool useTcp;
    int32_t code;
    uint64_t accessCode;

    std::string pluginName;
    std::string pluginFilePath;
    std::string passPhrase;
    NETID netid;
    std::string db;
    int verbose;
    int32_t retCode;

    CliPrintParams()
        : svcOrPlugin(0), port(DEF_PORT), useTcp(false), code(42), accessCode(42), netid(0), verbose(0), retCode(0)
    {

    }

    std::string toString() const {
        std::stringstream ss;
        switch (svcOrPlugin) {
            case 0:
                ss << _("Service: ") << address << ":" << port << " " << (useTcp ? "TCP" : "UDP")
                    << _(" code: ") << std::hex << code << _(" access code: ")  << accessCode;
                break;
            case 1:
                ss << _("Plugin: ") << pluginFilePath;
                break;
            default:
                ss << _("Identity source: ") << pluginName;
        }
        if (!db.empty())
            ss << _(". Database file name: ") << db;
        ss << _(". Pass-phrase: ") << passPhrase << _(". Verbose: ") << verbose << "\n";
        // for (auto & it : payload) ss << hexString(it) << "\n";
        return ss.str();
    }
};

static CliPrintParams params;

#define DLMT            ", "
#define TAB_DELIMITER   "\t"

static void printNetId(
    std::ostream &strm,
    const NETID &value,
    int verbosity
) {
    DEVADDR minAddr(value, false);
    DEVADDR maxAddr(value, true);

    if (verbosity > 0) {
        // print header
        strm
            << _("NetId") << TAB_DELIMITER
            << _("Type") << TAB_DELIMITER
            << _("Id") << TAB_DELIMITER
            << _("NwkId") << TAB_DELIMITER
            << _("DEVADDR min") << TAB_DELIMITER
            << _("DEVADDR max")
            << std::endl;
    }
    strm
        << value.toString() << TAB_DELIMITER
        << std::hex
        << (int) value.getType() << TAB_DELIMITER
        << value.getNetId() << TAB_DELIMITER
        << value.getNwkId() << TAB_DELIMITER
        << minAddr.toString() << TAB_DELIMITER
        << maxAddr.toString() << TAB_DELIMITER
        << std::endl;
}

static std::promise<const IdentityGetResponse *> promiseResponse;

class ResponseService : public ResponseClient {
public:
    void onError(
        QueryClient* client,
        const int32_t code,
        const int errorCode
    ) override {
        std::cerr << ERR_MESSAGE << code << _(", errno: ") << errorCode << "\n";
        params.retCode = code;
        promiseResponse.set_value(nullptr);
        client->stop();
    }

    void onIdentityGet(
        QueryClient* client,
        const IdentityGetResponse *response
    ) override {
        if (response) {
            if (!response->response.value.devid.empty())
                promiseResponse.set_value(response);
            else
                promiseResponse.set_value(nullptr);
        }
        client->stop();
    }

    void onIdentityOperation(
        QueryClient* client,
        const IdentityOperationResponse *response
    ) override {
        client->stop();
    }

    void onIdentityList(
        QueryClient* client,
        const IdentityListResponse *response
    ) override {
        client->stop();
    }

    void onGatewayOperation(
        QueryClient* client,
        const GatewayOperationResponse *response
    ) override {
        client->stop();
    }

    void onGatewayGet(
        QueryClient* client,
        const GatewayGetResponse *response
    ) override {
        client->stop();
    }

    void onGatewayList(
        QueryClient* client,
        const GatewayListResponse *response
    ) override {
        client->stop();
    }

    void onDisconnected(
        QueryClient* client
    ) override {
    }

    bool next(
        QueryClient *client
    ) {
        return false;
    }
};

static bool getDeviceByAddr(
    DEVICEID &retVal,
    const DEVADDR &devaddr
)
{
    // svcOrPlugin 0- service, 1- shared library plugin, 2- static (embedded) plugin
    switch (params.svcOrPlugin) {
        case 0: {
            ResponseService response;
            std::future<const IdentityGetResponse*> future = promiseResponse.get_future();
            auto c = UDPClient(params.address, params.port, &response);
            auto req = IdentityAddrRequest(QUERY_IDENTITY_EUI, devaddr, params.code, params.accessCode);
            c.request(&req);
            c.start();
            const IdentityGetResponse* r = future.get();
            if (r) {
                retVal = r->response;
                return true;
            } else
                return false;
        }
        case 1: {
            auto c = PluginClient(params.pluginFilePath);
            if (!c.svcIdentity || !c.svcGateway) {
                std::cerr
                    << ERR_MESSAGE << ERR_CODE_LOAD_PLUGINS_FAILED << ": "
                    << ERR_LOAD_PLUGINS_FAILED
                    << params.pluginFilePath
                    << std::endl;
                params.retCode = ERR_CODE_LOAD_PLUGINS_FAILED;
                return false;
            }
            if (!params.db.empty())
                c.svcIdentity->init(params.db, nullptr);
            else
                c.svcIdentity->init(params.passPhrase, &params.netid);
            return c.svcIdentity->get(retVal, devaddr) == 0;
        }
        default:
        {
            auto c = ServiceClient(params.pluginName);
            if (!params.db.empty())
                c.svcIdentity->init(params.db, nullptr);
            else
                c.svcIdentity->init(params.passPhrase, &params.netid);

            if (!c.svcIdentity || !c.svcGateway) {
                std::cerr
                    << ERR_MESSAGE << ERR_CODE_LOAD_PLUGINS_FAILED << ": "
                    << ERR_LOAD_PLUGINS_FAILED
                    << params.pluginName
                    << std::endl;
                params.retCode = ERR_CODE_LOAD_PLUGINS_FAILED;
                return false;
            }
            // 0- pass master key to generate keys
            c.svcIdentity->setOption(0, &params.passPhrase);
            return c.svcIdentity->get(retVal, devaddr) == 0;
        }
    }
}

static void printPacket(
    std::ostream &strm,
    const std::string &payload,
    int verbose
)
{
    auto sz = payload.size();
    if (verbose) {
        if (sz < SIZE_RFM_HEADER) {
            // try to print major header
            if (sz < SIZE_MHDR)
                return;
            auto mhdr = (MHDR *) &payload;
            strm << mtype2string((MTYPE) mhdr->f.mtype) << DLMT;
            return;
        }
        auto *rfm = (RFM_HEADER *) payload.c_str();
        strm << rfm_header2string(rfm) << DLMT
             << mac2string((void *) (payload.c_str() + SIZE_RFM_HEADER), rfm->fhdr.fctrl.f.foptslen, sz - SIZE_RFM_HEADER);

        DEVADDR a(rfm->fhdr.devaddr);

        if (hasFPort((void *) payload.c_str(), sz)) {
            strm << DLMT << (int) getFPort((void *) payload.c_str());
            char *pl = hasPayload((void *) payload.c_str(), sz);
            if (!pl)
                strm << DLMT << _("n/a");
            else {
                DEVICEID deviceId;
                std::string pld(pl, payloadSize((void *) payload.c_str(), sz));
                if (getDeviceByAddr(deviceId, rfm->fhdr.devaddr)) {
                    strm
                        << DLMT << _("fcnt: ") << (int) rfm->fhdr.fcnt
                        << DLMT << _("direction: ") << (int) (rfm->macheader.f.mtype & 1)
                        << DLMT << _("devAddr: ") << DEVADDR2string(rfm->fhdr.devaddr)
                        << DLMT << _("appSKey: ") << KEY2string(deviceId.id.appSKey);
                    decryptPayloadString(pld, rfm->fhdr.fcnt, rfm->macheader.f.mtype & 1, rfm->fhdr.devaddr, deviceId.id.appSKey);
                    strm << DLMT << DEVEUI2string(deviceId.id.devEUI) << DLMT
                         << hexString(pld);
                } else
                    strm << DLMT << _("n/a") << DLMT << hexString(pld);
            }
        }
        strm << std::endl;
        printNetId(strm, NETID(a.getNwkId()), verbose);
    } else {
        if (sz < SIZE_RFM_HEADER)
            return;
        auto *rfm = (RFM_HEADER *) payload.c_str();
        strm << mtype2string((MTYPE) rfm->macheader.f.mtype)
             << DLMT << DEVADDR2string(rfm->fhdr.devaddr);
        if (rfm->fhdr.fctrl.f.foptslen == 0)
            strm << DLMT << _("n/a");
        else
            strm << DLMT
                 << mac2string((void *) (payload.c_str() + SIZE_RFM_HEADER), rfm->fhdr.fctrl.f.foptslen, sz - SIZE_RFM_HEADER);

        if (!hasFPort((void *) payload.c_str(), sz))
            strm << DLMT << _("n/a");
        else
            strm << DLMT << (int) getFPort((void *) payload.c_str());
        char *pl = hasPayload((void *) payload.c_str(), sz);
        if (!pl)
            strm << DLMT << _("n/a");
        else {
            DEVICEID deviceId;
            std::string pld(pl, sz - (pl - (char *) rfm) - SIZE_MIC);
            if (getDeviceByAddr(deviceId, rfm->fhdr.devaddr)) {
                decryptPayloadString(pld, rfm->fhdr.fcnt, rfm->macheader.f.mtype & 1, rfm->fhdr.devaddr, deviceId.id.appSKey);
                strm << DLMT << DEVEUI2string(deviceId.id.devEUI) << DLMT
                     << hexString(pld);
            } else
                strm << DLMT << _("n/a") << DLMT << hexString(pld);
        }
    }
}

static void run()
{
    for (const auto& p : params.payload) {
        printPacket(std::cout, p, params.verbose);
        std::cout << std::endl;
    }
}

// for shared library use "storage-gen:Gen:Memory"
#define DEF_PLUGIN  "json"
#define DEF_MASTERKEY   "masterkey"

int main(int argc, char **argv) {
    struct arg_str *a_hex = arg_strn(nullptr, nullptr, _("<hex>"),  1, 100, _("payload"));
    struct arg_str *a_service_n_port = arg_str0("s", "service", _("<address:port>"), "");
    struct arg_str *a_plugin_file_n_class = arg_str0("p", "plugin", _("<plugin>"), _("Default " DEF_PLUGIN));
    struct arg_str *a_db = arg_str0("f", "db", _("<database file>"), _("database file name. Default none"));
    struct arg_str* a_pass_phrase = arg_str0("m", "masterkey", _("<pass-phrase>"), _("Default " DEF_MASTERKEY));
    struct arg_str *a_net_id = arg_str0("n", "network-id", _("<hex|hex:hex>"), _("Hexadecimal <network-id> or <net-type>:<net-id>. Default 0"));
    struct arg_int *a_code = arg_int0("c", "code", _("<number>"), _("Default 42. 0x - hex number prefix"));
    struct arg_str *a_access_code = arg_str0("a", "access", _("<hex>"), _("Default 2a (42 decimal)"));
    struct arg_lit *a_tcp = arg_lit0("t", "tcp", _("use TCP protocol. Default UDP"));
    struct arg_lit *a_verbose = arg_litn("v", "verbose", 0, 2, _("-v verbose -vv debug"));
    struct arg_lit *a_help = arg_lit0("h", "help", _("Show this help"));
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = {
		a_hex, a_service_n_port, a_plugin_file_n_class, a_db, a_pass_phrase, a_net_id,
        a_code, a_access_code, a_tcp, a_verbose,
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

    if (!a_help->count) {
        if (a_db->count)
            params.db = *a_db->sval;
        if (a_pass_phrase->count)
            params.passPhrase = *a_pass_phrase->sval;
        else
            params.passPhrase = DEF_MASTERKEY;
        if (a_net_id)
            readNetId(params.netid, *a_net_id->sval);

        if (a_service_n_port->count) {
            if (!splitAddress(params.address, params.port, std::string(*a_service_n_port->sval))) {
                std::cerr << _("Invalid service address:port ") << *a_service_n_port->sval << std::endl;
                errorCount++;
            }
            params.useTcp = a_tcp->count > 0;
            params.svcOrPlugin = 0;
        } else {
            // try load shared library
            if (!params.pluginFilePath.empty()) {
                if (ServiceClient::hasStaticPlugin(params.pluginFilePath)) {
                    // "load" from static by name: "gen", "mem", "sqlite"
                    params.pluginName = params.pluginFilePath;
                    params.svcOrPlugin = 2;
                } else {
                    params.pluginName = "";
                    params.svcOrPlugin = 1;
                }
            } else {
                params.svcOrPlugin = 2;
                params.pluginName = DEF_PLUGIN;
            }
        }
    }

    if (a_access_code->count)
        params.accessCode = strtoull(*a_access_code->sval, nullptr, 16);
    else
        params.accessCode = 42;

    if (a_code->count)
        params.code = *a_code->ival;
    else
        params.code = 42;

    for (int i = 0; i < a_hex->count; i++) {
        params.payload.push_back(hex2string(a_hex->sval[i]));
    }

    // special case: '--help' takes precedence over error reporting
	if ((a_help->count) || errorCount) {
		if (errorCount)
			arg_print_errors(stderr, a_end, programName);
		std::cerr << _("Usage: ") << programName << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << _("Print LoRaWAN packet") << std::endl;
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
