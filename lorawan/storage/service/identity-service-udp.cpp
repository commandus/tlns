#include <iostream>

#include "lorawan/helper/ip-address.h"
#include "lorawan/storage/service/identity-service-udp.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/file-helper.h"
#include "lorawan/storage/client/udp-client.h"
#include "lorawan/storage/serialization/gateway-binary-serialization.h"

#ifdef ESP_PLATFORM
#include <iostream>
#include "platform-defs.h"
#endif

/*
IdentityQueryTag
QUERY_IDENTITY_ADDR = 'a',
QUERY_IDENTITY_EUI = 'i',
QUERY_IDENTITY_LIST = 'l',
QUERY_IDENTITY_COUNT = 'c',
QUERY_IDENTITY_ASSIGN = 'p',
QUERY_IDENTITY_RM = 'r',
QUERY_IDENTITY_FORCE_SAVE = 's',
QUERY_IDENTITY_CLOSE_RESOURCES = 'e'
*/

class ResponseService : public ResponseClient {
public:
    ClientUDPIdentityService *svc;
    explicit ResponseService(
        ClientUDPIdentityService *aSvc
    )
        : svc(aSvc)
    {
    }

    void onError(
        QueryClient* client,
        const int32_t code,
        const int errorCode
    ) override {
        std::cerr << ERR_MESSAGE << code << ", errno: " << errorCode << "\n";
        client->stop();
        svc->retCode = code;
    }

    void onIdentityGet(
        QueryClient* client,
        const IdentityGetResponse *response
    ) override {
        if (response) {
            if (!response->response.value.devid.empty()) {
                if (svc->verbose)
                    std::cout << response->toJsonString() << std::endl;
                else
                    std::cout << response->response.toString() << std::endl;
            }
        }
    }

    void onIdentityOperation(
        QueryClient* client,
        const IdentityOperationResponse *response
    ) override {
        if (response) {
            if (svc->verbose) {
                if (response->response != 0)
                    std::cout << response->toJsonString() << std::endl;
                else {
                    std::cerr << identityTag2string((IdentityQueryTag) response->tag)
                              // << " " << (int) response->size
                              << " completed\n";
                }
            } else {
                if (response->response != 0) {
                    std::cerr << ERR_MESSAGE << response->response << std::endl;
                }
            }
        }
    }

    void onIdentityList(
        QueryClient* client,
        const IdentityListResponse *response
    ) override {
        if (response) {
            if (svc->verbose) {
                std::cout << response->toJsonString() << std::endl;
            } else {
                for (auto i = 0; i < response->response; i++) {
                    std::cout << response->identities[i].toString() << std::endl;
                }
            }
        }
    }

    void onGatewayOperation(
        QueryClient* client,
        const GatewayOperationResponse *response
    ) override {
        if (response) {
            if (svc->verbose) {
                if (response->response != 0)
                    std::cout << response->toJsonString() << std::endl;
                else {
                    std::cerr << gatewayTag2string((GatewayQueryTag) response->tag)
                              // << " " << (int) response->size
                              << " completed\n";
                }
            } else {
                if (response->response != 0) {
                    std::cerr << ERR_MESSAGE << response->response << std::endl;
                }
            }
        }
    }

    void onGatewayGet(
        QueryClient* client,
        const GatewayGetResponse *response
    ) override {
        if (response) {
            if (svc->verbose)
                std::cout << response->toJsonString() << std::endl;
            else
            if (isIP(&response->response.sockaddr) && response->response.gatewayId) {
                std::cout
                    << std::hex << response->response.gatewayId << "\t"
                    << sockaddr2string(&response->response.sockaddr) << std::endl;
            }
        }
    }

    void onGatewayList(
        QueryClient* client,
        const GatewayListResponse *response
    ) override {
        if (response) {
            if (svc->verbose) {
                std::cout << response->toJsonString() << std::endl;
            } else {
                for (uint32_t i = 0; i < response->response; i++) {
                    std::cout << std::hex << response->identities[i].gatewayId
                        << "\t" << sockaddr2string(&response->identities[i].sockaddr)
                        << std::endl;
                }
            }
        }
    }

    void onDisconnected(
        QueryClient* client
    ) override {
        if (svc->verbose) {
            std::cerr << "disconnected \n";
        }
    }
};

ClientUDPIdentityService::ClientUDPIdentityService()
    :  port(0), code(0), accessCode(0), client(nullptr), retCode(CODE_OK), verbose(0)
{
}

ClientUDPIdentityService::~ClientUDPIdentityService()
{
    if (client) {
        delete client;
        client = nullptr;
    }
}

// synchronous calls
/**
 * request device identifier by network address. Return 0 if success, retval = EUI and keys
 * @param retval device identifier
 * @param devaddr network address
 * @return CODE_OK- success
 */
int ClientUDPIdentityService::get(
    DEVICEID &retVal,
    const DEVADDR &addr
)
{
    IdentityAddrRequest req(QUERY_IDENTITY_EUI, addr, code, accessCode);
    syncClient.request(&req);
    return CODE_OK;
}

// List entries
int ClientUDPIdentityService::list(
    std::vector<NETWORKIDENTITY> &retVal,
    uint32_t offset,
    uint8_t size
) {
    IdentityOperationRequest req(QUERY_IDENTITY_LIST, offset, size, code, accessCode);
    syncClient.request(&req);
    return CODE_OK;
}

// Entries count
size_t ClientUDPIdentityService::size()
{
    IdentityOperationRequest req(QUERY_IDENTITY_COUNT, 0, 0, code, accessCode);
    syncClient.request(&req);
    return CODE_OK;
}

/**
* request network identity(with address) by network address. Return 0 if success, retval = EUI and keys
* @param retval network identity(with address)
* @param eui device EUI
* @return CODE_OK- success
*/
int ClientUDPIdentityService::getNetworkIdentity(
    NETWORKIDENTITY &retVal,
    const DEVEUI &devEUI
)
{
    IdentityEUIRequest req(QUERY_IDENTITY_ADDR, devEUI, code, accessCode);
    syncClient.request(&req);
    return CODE_OK;
}

/**
 * UPSERT SQLite >= 3.24.0
 * @param request gateway identifier or address
 * @return 0- success
 */
int ClientUDPIdentityService::put(
    const DEVADDR &devAddr,
    const DEVICEID &devId
)
{
    IdentityAssignRequest req(QUERY_IDENTITY_ASSIGN, NETWORKIDENTITY(devAddr, devId), code, accessCode);
    syncClient.request(&req);
    return CODE_OK;
}

int ClientUDPIdentityService::rm(
    const DEVADDR &devAddr
)
{
    IdentityAddrRequest req(QUERY_IDENTITY_RM, devAddr, code, accessCode);
    syncClient.request(&req);
    return CODE_OK;
}

int ClientUDPIdentityService::init(
    const std::string &addrPort,
    void *database
)
{
    splitAddress(addr, port, addrPort);
    ResponseService onResp(this);

#ifdef ENABLE_LIBUV
    client = new UvClient(params.useTcp, params.address, params.port, &onResp);
#else
    client = new UDPClient(addr, port, &onResp);
#endif
    // request first address to resolve
    client->start();
    return CODE_OK;
}

void ClientUDPIdentityService::flush()
{
}

void ClientUDPIdentityService::done()
{
    if (client) {
        delete client;
        client = nullptr;
    }
}

/**
 * Return next network address if available
 * @return 0- success, ERR_CODE_ADDR_SPACE_FULL- no address available
 */
int ClientUDPIdentityService::next(
    NETWORKIDENTITY &retval
)
{
    IdentityOperationRequest req(QUERY_IDENTITY_NEXT, 0, 0, code, accessCode);
    syncClient.request(&req);
    return CODE_OK;
}

void ClientUDPIdentityService::setOption(
    int option,
    void *value
)
{
    if (!value)
        return;
    switch (option) {
        case 1:
            code = * (int32_t *)value;
            break;
        case 2:
            accessCode = * (uint64_t *) value;
            break;
        default:
            break;
    }
}

// ------------------- asynchronous calls -------------------

int ClientUDPIdentityService::cGet(const DEVADDR &request)
{
    IdentityAddrRequest req(QUERY_IDENTITY_EUI, addr, code, accessCode);
    client->request(&req);
    return CODE_OK;
}

int ClientUDPIdentityService::cGetNetworkIdentity(const DEVEUI &devEUI)
{
    IdentityEUIRequest req(QUERY_IDENTITY_ADDR, devEUI, code, accessCode);
    client->request(&req);
    return CODE_OK;
}

int ClientUDPIdentityService::cPut(const DEVADDR &devAddr, const DEVICEID &devId)
{
    IdentityAssignRequest req(QUERY_IDENTITY_ASSIGN, NETWORKIDENTITY(devAddr, devId), code, accessCode);
    client->request(&req);
    return CODE_OK;
}

int ClientUDPIdentityService::cRm(const DEVADDR &devAddr)
{
    IdentityAddrRequest req(QUERY_IDENTITY_RM, devAddr, code, accessCode);
    client->request(&req);
    return CODE_OK;
}

int ClientUDPIdentityService::cList(
    uint32_t offset,
    uint8_t size
)
{
    IdentityOperationRequest req(QUERY_IDENTITY_LIST, offset, size, code, accessCode);
    client->request(&req);
    return CODE_OK;
}

int ClientUDPIdentityService::cSize()
{
    IdentityOperationRequest req(QUERY_IDENTITY_COUNT, 0, 0, code, accessCode);
    client->request(&req);
    return CODE_OK;
}

int ClientUDPIdentityService::cNext()
{
    IdentityOperationRequest req(QUERY_IDENTITY_NEXT, 0, 0, code, accessCode);
    client->request(&req);
    return CODE_OK;
}

int ClientUDPIdentityService::filter(
    std::vector<NETWORKIDENTITY> &retVal,
    const std::vector<NETWORK_IDENTITY_FILTER> &filters,
    uint32_t offset,
    uint8_t size
)
{
    IdentityOperationRequest req(QUERY_IDENTITY_FILTER, offset, size, code, accessCode);
    syncClient.request(&req);
    return CODE_OK;
}

int ClientUDPIdentityService::cFilter(
    const std::vector<NETWORK_IDENTITY_FILTER> &filters,
    uint32_t offset,
    uint8_t size
)
{
    IdentityOperationRequest req(QUERY_IDENTITY_FILTER, offset, size, code, accessCode);
    client->request(&req);
    return CODE_OK;
}

EXPORT_SHARED_C_FUNC IdentityService* makeIdentityService4()
{
    return new ClientUDPIdentityService;
}
