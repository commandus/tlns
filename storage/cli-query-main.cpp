#include <string>
#include <iostream>

#include <sstream>
#include <cstring>

#include "argtable3/argtable3.h"

#ifdef ENABLE_LIBUV
#include <uv.h>
#include "lorawan/storage/client/uv-client.h"
#else
#include "lorawan/storage/client/udp-client.h"
#endif
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-msg.h"
#include "lorawan/lorawan-string.h"
#include "log.h"
#include "lorawan/storage/serialization/identity-binary-serialization.h"
#include "lorawan/storage/serialization/gateway-binary-serialization.h"
#include "lorawan/helper/ip-address.h"
#include "lorawan/helper/ip-helper.h"
#include "cli-helper.h"

// i18n
// #include <libintl.h>
// #define _(String) gettext (String)
#define _(String) (String)

const char *programName = "lorawan-identity-query";
#define DEF_PORT 4244

// global parameters
class CliQueryParams {
public:
    char tag;
    std::vector<DeviceOrGatewayIdentity> query;
    uint32_t queryPos;
    bool useTcp;
    int verbose;
    std::string intf;
    uint16_t port;
    int32_t code;
    uint64_t accessCode;
    uint32_t offset;
    uint8_t size;

    int32_t retCode;

    CliQueryParams()
        : tag(QUERY_GATEWAY_NONE), queryPos(0), useTcp(false), verbose(0), port(DEF_PORT), code(42), accessCode(42), offset(0), size(0),
          retCode(0)
    {

    }

    std::string toString() {
        std::stringstream ss;
        ss
            << _("Service: ") << intf << ":" << port << " " << (useTcp ? "TCP" : "UDP") << " "
            << _("command: ") << commandLongName(tag) << _(", code: ") << std::hex << code << _(", access code: ")  << accessCode << " "
            << _("offset: ") << std::dec << offset << _(", size: ")  << (int) size << "\n";
        for (auto & it : query) {
            if (it.hasDevice) {
                if (!it.nid.value.devaddr.empty())
                    ss << DEVADDR2string(it.nid.value.devaddr);
                ss << "\t" << it.nid.value.devid.toString() << "\n";
            }
            if (it.hasGateway) {
                if (it.gid.gatewayId)
                    ss << std::hex << it.gid.gatewayId;
                ss << "\t" << sockaddr2string(&it.gid.sockaddr) << "\n";
            }
        }
        return ss.str();
    }
};

static CliQueryParams params;

class ResponseService : public ResponseClient {
public:
    const std::vector<DeviceOrGatewayIdentity> &query;
    int verbose;
    explicit ResponseService(
        const std::vector<DeviceOrGatewayIdentity> &aQuery,
        int aVerbose
    )
        : query(aQuery), verbose(aVerbose)
    {
    }

    void onError(
        QueryClient* client,
        const int32_t code,
        const int errorCode
    ) override {
        std::cerr << ERR_MESSAGE << code << _(", errno: ") << errorCode << "\n";
        client->stop();
        params.retCode = code;
    }

    void onIdentityGet(
        QueryClient* client,
        const IdentityGetResponse *response
    ) override {
        if (response) {
            if (!response->response.value.devid.empty()) {
                if (params.verbose)
                    std::cout << response->toJsonString() << std::endl;
                else
                    std::cout << response->response.toString() << std::endl;
            }
            if (!next(client)) {
                client->stop();
            }
        } else {
            client->stop();
        }
    }

    void onIdentityOperation(
        QueryClient* client,
        const IdentityOperationResponse *response
    ) override {
        if (response) {
            if (params.verbose) {
                if (response->response != 0)
                    std::cout << response->toJsonString() << std::endl;
                else {
                    std::cerr << identityTag2string((IdentityQueryTag) response->tag)
                      // << " " << (int) response->size
                      << _(" completed\n");
                }
            } else {
                if (response->response != 0) {
                    std::cerr << ERR_MESSAGE << response->response << std::endl;
                }
            }
            if (!next(client)) {
                client->stop();
            }
        } else {
            client->stop();
        }
    }

    void onIdentityList(
        QueryClient* client,
        const IdentityListResponse *response
    ) override {
        if (response) {
            if (params.verbose) {
                std::cout << response->toJsonString() << std::endl;
            } else {
                for (auto i = 0; i < response->response; i++) {
                    std::cout << response->identities[i].toString() << std::endl;
                }
            }
            if (!next(client)) {
                client->stop();
            }
        } else {
            client->stop();
        }
    }

    void onGatewayOperation(
        QueryClient* client,
        const GatewayOperationResponse *response
    ) override {
        if (response) {
            if (params.verbose) {
                if (response->response != 0)
                    std::cout << response->toJsonString() << std::endl;
                else {
                    std::cerr << gatewayTag2string((GatewayQueryTag) response->tag)
                    // << " " << (int) response->size
                    << _(" completed\n");
                }
            } else {
                if (response->response != 0) {
                    std::cerr << ERR_MESSAGE << response->response << std::endl;
                }
            }
            if (!next(client)) {
                client->stop();
            }
        } else {
            client->stop();
        }
    }

    void onGatewayGet(
        QueryClient* client,
        const GatewayGetResponse *response
    ) override {
        if (response) {
            if (params.verbose)
                std::cout << response->toJsonString() << std::endl;
            else
                if (isIP(&response->response.sockaddr) && response->response.gatewayId) {
                    std::cout
                        << std::hex << response->response.gatewayId << "\t"
                        << sockaddr2string(&response->response.sockaddr) << std::endl;
                }
            if (!next(client)) {
                client->stop();
            }
        } else {
            client->stop();
        }
	}

    void onGatewayList(
        QueryClient* client,
        const GatewayListResponse *response
    ) override {
        if (response) {
            if (params.verbose) {
                std::cout << response->toJsonString() << std::endl;
            } else {
                for (uint32_t i = 0; i < response->response; i++) {
                    std::cout << std::hex << response->identities[i].gatewayId
                        << "\t" << sockaddr2string(&response->identities[i].sockaddr)
                        << std::endl;
                }
            }
            if (!next(client)) {
                client->stop();
            }
        } else {
            client->stop();
        }
    }

    void onDisconnected(
            QueryClient* client
    ) override {
        if (verbose) {
            std::cerr << _("disconnected\n");
        }
    }

    bool next(
        QueryClient *client
    ) {
        bool hasNext = params.queryPos < query.size();
        ServiceMessage *req = nullptr;
        if (hasNext) {
            DeviceOrGatewayIdentity id = params.query[params.queryPos];
            switch (params.tag) {
                case QUERY_IDENTITY_LIST:
                    req = new IdentityOperationRequest(params.tag, params.offset, params.size, params.code, params.accessCode);
                    break;
                case QUERY_IDENTITY_COUNT:
                    req = new IdentityOperationRequest(params.tag, params.offset, params.size, params.code, params.accessCode);
                    break;
                case QUERY_IDENTITY_NEXT:
                    req = new IdentityOperationRequest(params.tag, params.offset, params.size, params.code, params.accessCode);
                    break;
                case QUERY_IDENTITY_ASSIGN:
                    req = new IdentityAssignRequest(params.tag, id.nid, params.code, params.accessCode);
                    break;
                case QUERY_IDENTITY_RM:
                    req = new IdentityAddrRequest(params.tag, id.nid.value.devaddr, params.code, params.accessCode);
                    break;
                case QUERY_IDENTITY_FORCE_SAVE:
                    break;
                case QUERY_IDENTITY_CLOSE_RESOURCES:
                    break;
                case QUERY_IDENTITY_EUI:
                    req = new IdentityAddrRequest(QUERY_IDENTITY_EUI, id.nid.value.devaddr, params.code, params.accessCode);
                    break;
                case QUERY_IDENTITY_ADDR:
                    req = new IdentityEUIRequest(params.tag, id.nid.value.devid.id.devEUI, params.code, params.accessCode);
                    break;
                // gateway
                case QUERY_GATEWAY_LIST:
                    req = new GatewayOperationRequest(params.tag, params.offset, params.size, params.code, params.accessCode);
                    break;
                case QUERY_GATEWAY_COUNT:
                    req = new GatewayOperationRequest(params.tag, params.offset, params.size, params.code, params.accessCode);
                    break;
                case QUERY_GATEWAY_ASSIGN:
                    req = new GatewayIdAddrRequest(params.tag, id.gid, params.code, params.accessCode);
                    break;
                case QUERY_GATEWAY_RM:
                    req = new GatewayIdAddrRequest(params.tag, id.gid, params.code, params.accessCode);
                    break;
                case QUERY_GATEWAY_FORCE_SAVE:
                    break;
                case QUERY_GATEWAY_CLOSE_RESOURCES:
                    break;
                case QUERY_GATEWAY_NONE:
                    break;
                case QUERY_GATEWAY_ID:
                case QUERY_GATEWAY_ADDR:
                    if (id.gid.gatewayId == 0)
                        req = new GatewayAddrRequest(id.gid.sockaddr, params.code, params.accessCode);
                    else
                        req = new GatewayIdRequest((char) params.tag, id.gid.gatewayId, params.code, params.accessCode);
                    break;
            }
        }
        if (req) {
            ServiceMessage *previousMessage = client->request(req);
            if (previousMessage)
                delete previousMessage;
        }
        params.queryPos++;
        return hasNext;
    }
};

static void run()
{
	ResponseService onResp(params.query, params.verbose);
    QueryClient *client;
#ifdef ENABLE_LIBUV
    client = new UvClient(params.useTcp, params.address, params.port, &onResp);
#else
    client = new UDPClient(params.intf, params.port, &onResp);
#endif
    // request first address to resolve
    if (!onResp.next(client)) {
        params.retCode = ERR_CODE_PARAM_INVALID;
        return;
    }
    client->start();
    delete client;
}

int main(int argc, char **argv) {
    std::string shortCL = shortCommandList('|');
    struct arg_str *a_query = arg_strn(nullptr, nullptr, _("<command | id | address:port>"), 1, 100,
        shortCL.c_str());
    struct arg_str *a_interface_n_port = arg_str0("s", "service", _("<ipaddr:port>"), _("Default localhost:4244"));
    struct arg_int *a_code = arg_int0("c", "code", _("<number>"), _("Default 42. 0x - hex number prefix"));
    struct arg_str *a_access_code = arg_str0("a", "access", _("<hex>"), _("Default 2a (42 decimal)"));
	struct arg_lit *a_tcp = arg_lit0("t", "tcp", _("use TCP protocol. Default UDP"));
    struct arg_int *a_offset = arg_int0("o", "offset", _("<0..>"), _("list offset. Default 0. Max 4294967295"));
    struct arg_int *a_size = arg_int0("z", "size", _("<number>"), _("list size limit. Default 10. Max 255"));
    struct arg_lit *a_verbose = arg_litn("v", "verbose", 0, 2, _("-v verbose -vv debug"));
    struct arg_lit *a_help = arg_lit0("h", "help", _("Show this help"));
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_query, a_interface_n_port,
        a_code, a_access_code, a_tcp,
        a_offset, a_size, a_verbose,
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
    params.useTcp = a_tcp->count > 0;

    if (a_interface_n_port->count) {
        if (!splitAddress(params.intf, params.port, std::string(*a_interface_n_port->sval))) {
            return ERR_CODE_COMMAND_LINE;
        }
    } else {
        params.intf = "localhost";
        params.port = DEF_PORT;
    }

    params.tag = QUERY_IDENTITY_ADDR;

    params.query.reserve(a_query->count);
    bool queryHasIdentity = false;
    bool queryHasGateway = false;
    for (int i = 0; i < a_query->count; i++) {
        enum IdentityQueryTag identityQueryTag = isIdentityTag(a_query->sval[i]);
        if (identityQueryTag != QUERY_IDENTITY_NONE) {
            params.tag = identityQueryTag;
            queryHasIdentity = true;
            continue;
        }
        enum GatewayQueryTag gatewayTag = isGatewayTag(a_query->sval[i]);
        if (gatewayTag != QUERY_GATEWAY_NONE) {
            params.tag = gatewayTag;
            queryHasGateway = true;
            continue;
        }
        if (queryHasIdentity) {
            DeviceOrGatewayIdentity id;
            id.hasDevice = true;
            switch (params.tag) {
                case QUERY_IDENTITY_ADDR:
                    string2DEVEUI(id.nid.value.devid.id.devEUI, a_query->sval[i]);
                    break;
                case QUERY_IDENTITY_EUI:
                    string2DEVADDR(id.nid.value.devaddr, a_query->sval[i]);
                    break;
                default:
                    if (!string2NETWORKIDENTITY(id.nid, a_query->sval[i])) {
                        return ERR_CODE_PARAM_INVALID;
                    }
            }
            params.query.push_back(id);
        }
        if (queryHasGateway) {
            std::string a;
            uint16_t p;
            DeviceOrGatewayIdentity id;
            id.hasGateway = true;
            if (splitAddress(a, p, a_query->sval[i])) {
                // IP address
                string2sockaddr(&id.gid.sockaddr, a, p);
            } else {
                char *last;
                id.gid.gatewayId = strtoull(a_query->sval[i], &last, 16);
                if (*last)
                    return ERR_CODE_COMMAND_LINE;
            }
            params.query.push_back(id);
        }
    }

    if (params.tag == QUERY_IDENTITY_LIST || params.tag == QUERY_GATEWAY_LIST) {
        DeviceOrGatewayIdentity id;
        if (params.query.empty()) {
            params.query.push_back(id);
        }
        if (a_offset->count) {
            params.offset = (size_t) *a_offset->ival;
        }
        if (a_size->count) {
            auto v = *a_size->ival;
            if (v < 0)
                v = 10;
            if (v > 255)
                v = 255;
            params.size = v;
        }
        else
            params.size = 10;
    }

    if (params.tag == QUERY_GATEWAY_ASSIGN) {
        // reorder query
        mergeIdAddress(params.query);
    }

    if (a_access_code->count)
        params.accessCode = strtoull(*a_access_code->sval, nullptr, 16);
    else
        params.accessCode = 42;

    if (a_code->count)
        params.code = *a_code->ival;
    else
        params.code = 42;

    // special case: '--help' takes precedence over error reporting
	if ((a_help->count) || errorCount) {
		if (errorCount)
			arg_print_errors(stderr, a_end, programName);
		std::cerr << _("Usage: ") << programName << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << _("LoRaWAN storage query") << std::endl;
		arg_print_glossary(stderr, argtable, "  %-27s %s\n");
        std::cerr << _("Commands:\n") << listCommands() << std::endl;
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
