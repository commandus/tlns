#include "plugin-query-client.h"

#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/lorawan-msg.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#define dlopen(fileName, opt) LoadLibraryA(fileName)
#define dlclose FreeLibrary
#define dlsym GetProcAddress
#else
#include <dlfcn.h>
#include <algorithm>
#endif

#include "lorawan/storage/serialization/identity-binary-serialization.h"
#include "lorawan/storage/serialization/gateway-binary-serialization.h"

#ifdef ENABLE_DEBUG
#include <iostream>
#include <cstring>
#include "lorawan-msg.h"
#endif

static void parseResponse(
    PluginQueryClient *client,
    const unsigned char *buf,
    ssize_t nRead
) {
    if (!client)
        return;
    if (isIdentityTag(buf, nRead)) {
        enum IdentityQueryTag tag = validateIdentityQuery(buf, nRead);
        switch (tag) {
            case QUERY_IDENTITY_EUI:   // request gateway identifier(with address) by network address.
            case QUERY_IDENTITY_ADDR:   // request gateway address (with identifier) by identifier.
            {
                IdentityGetResponse gr(buf, nRead);
                gr.ntoh();
                client->onResponse->onIdentityGet(client, &gr);
            }
                break;
            case QUERY_IDENTITY_LIST:   // List entries
            {
                IdentityListResponse gr(buf, nRead);
                gr.response = NTOH4(gr.response);
                gr.ntoh();
                client->onResponse->onIdentityList(client, &gr);
            }
                break;
            default: {
                IdentityOperationResponse gr(buf, nRead);
                gr.ntoh();
                client->onResponse->onIdentityOperation(client, &gr);
            }
                break;
        }

    } else {
        enum GatewayQueryTag tag = validateGatewayQuery(buf, nRead);
        switch (tag) {
            case QUERY_GATEWAY_ADDR:   // request gateway identifier(with address) by network address.
            case QUERY_GATEWAY_ID:   // request gateway address (with identifier) by identifier.
            {
                GatewayGetResponse gr(buf, nRead);
                gr.ntoh();
                client->onResponse->onGatewayGet(client, &gr);
            }
                break;
            case QUERY_GATEWAY_LIST:   // List entries
            {
                GatewayListResponse gr(buf, nRead);
                gr.response = NTOH4(gr.response);
                gr.ntoh();
                client->onResponse->onGatewayList(client, &gr);
            }
                break;
            default: {
                GatewayOperationResponse gr(buf, nRead);
                gr.ntoh();
                client->onResponse->onGatewayOperation(client, &gr);
            }
                break;
        }
    }
}

typedef IdentityService*(*makeIdentityServiceFunc)();
typedef GatewayService*(*makeGatewayServiceFunc)();

const std::string MAKE_FUNC_PREFIX = "make";
const std::string MAKE_FUNC_IDENTITY_SUFFIX = "IdentityService";
const std::string MAKE_FUNC_GATEWAY_SUFFIX = "GatewayService";

int PluginQueryClient::load(
    const std::string &fileName,
    const std::string &identityClassName,
    const std::string &gatewayClassName
)
{
    std::string makeIdentityClass = MAKE_FUNC_PREFIX + identityClassName + MAKE_FUNC_IDENTITY_SUFFIX;
    std::string makeGatewayClass = MAKE_FUNC_PREFIX + gatewayClassName + MAKE_FUNC_GATEWAY_SUFFIX;

    handleSvc = dlopen(fileName.c_str(), RTLD_LAZY);
    if (handleSvc) {
        makeIdentityServiceFunc fI = (makeIdentityServiceFunc) dlsym(handleSvc, makeIdentityClass.c_str());
        makeGatewayServiceFunc fG = (makeGatewayServiceFunc) dlsym(handleSvc, makeGatewayClass.c_str());
        if (fI && fG) {
            svcIdentity = fI();
            svcGateway = fG();
            return CODE_OK;
        }
    }
    return ERR_CODE_LOAD_PLUGINS_FAILED;
}

void PluginQueryClient::unload()
{
    delete svcIdentity;
    svcIdentity = nullptr;
    delete svcGateway;
    svcGateway = nullptr;

    if (handleSvc) {
        dlclose(handleSvc);
        handleSvc = nullptr;
    }
}

void PluginQueryClient::stop()
{
    status = ERR_CODE_STOPPED;
}

PluginQueryClient::PluginQueryClient(
        const std::string &fileName,
        const std::string &identityClassName,
        const std::string &gatewayClassName,
        ResponseClient *aOnResponse,
        int32_t aCode,
        uint64_t aAccessCode
)
	: QueryClient(aOnResponse), handleSvc(nullptr), svcIdentity(nullptr), svcGateway(nullptr),
    status(CODE_OK), query(nullptr), code(aCode), accessCode(aAccessCode)
{
    load(fileName, identityClassName, gatewayClassName);
}

PluginQueryClient::~PluginQueryClient()
{
    stop();
    unload();
}

ServiceMessage* PluginQueryClient::request(
    ServiceMessage* value
)
{
    ServiceMessage* r = query;
    query = value;
    return r;
}

void PluginQueryClient::start() {
    IdentityBinarySerialization identitySerialization(svcIdentity, code, accessCode);
    GatewayBinarySerialization gatewaySerialization(svcGateway, code, accessCode);
    while (status != ERR_CODE_STOPPED) {
        if (!query) {
            status = ERR_CODE_STOPPED;
            break;
        }
        unsigned char rBuf[307];
        unsigned char qBuf[307];
        size_t qSize = query->serialize(qBuf);
        size_t sz = identitySerialization.query(rBuf, sizeof(rBuf), qBuf, qSize);
        if (sz == 0) {
            sz = gatewaySerialization.query(rBuf, sizeof(rBuf), qBuf, qSize);
        }
        if (isIdentityTag(rBuf, sz)) {
            enum IdentityQueryTag tag = validateIdentityQuery(rBuf, sz);
            switch (tag) {
                case QUERY_IDENTITY_EUI:   // request gateway identifier(with address) by network address.
                case QUERY_IDENTITY_ADDR:   // request gateway address (with identifier) by identifier.
                {
                    IdentityGetResponse gr(rBuf, sz);
                    gr.ntoh();
                    onResponse->onIdentityGet(this, &gr);
                }
                    break;
                case QUERY_IDENTITY_LIST:   // List entries
                {
                    IdentityListResponse gr(rBuf, sz);
                    gr.response = NTOH4(gr.response);
                    gr.ntoh();
                    onResponse->onIdentityList(this, &gr);
                }
                    break;
                default: {
                    IdentityOperationResponse gr(rBuf, sz);
                    gr.ntoh();
                    onResponse->onIdentityOperation(this, &gr);
                }
                    break;
            }
        } else {
            enum GatewayQueryTag tag = validateGatewayQuery(rBuf, sz);
            switch (tag) {
                case QUERY_GATEWAY_ADDR:   // request gateway identifier(with address) by network address.
                case QUERY_GATEWAY_ID:   // request gateway address (with identifier) by identifier.
                {
                    GatewayGetResponse gr(rBuf, sz);
                    gr.ntoh();
                    onResponse->onGatewayGet(this, &gr);
                }
                    break;
                case QUERY_GATEWAY_LIST:   // List entries
                {
                    GatewayListResponse gr(rBuf, sz);
                    gr.response = NTOH4(gr.response);
                    gr.ntoh();
                    onResponse->onGatewayList(this, &gr);
                }
                    break;
                default: {
                    GatewayOperationResponse gr(rBuf, sz);
                    gr.ntoh();
                    onResponse->onGatewayOperation(this, &gr);
                }
                    break;
            }
        }
    }
}
