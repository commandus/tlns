#include <cstring>
#include <fstream>
#include <iostream>

#include "gateway-service-json.h"
#include "lorawan/helper/ip-address.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"
#include "nlohmann/json.hpp"

#ifdef ESP_PLATFORM
#include <iostream>
#include "platform-defs.h"
#endif

JsonGatewayService::JsonGatewayService() = default;

JsonGatewayService::~JsonGatewayService() = default;


/**
 * request device identifier by network address. Return 0 if success, retval = EUI and keys
 * @param retval device identifier
 * @param devaddr network address
 * @return LORA_OK- success
 */
int JsonGatewayService::get(
    GatewayIdentity &retVal,
    const GatewayIdentity &request
)
{
    if (request.gatewayId) {
        // find out by gateway identifier
        auto r = storage.find(request.gatewayId);
        if (r != storage.end()) {
            retVal = r->second;
            return CODE_OK;
        } else {
            memset(&retVal.sockaddr, 0, sizeof(retVal.sockaddr));
            return ERR_CODE_GATEWAY_NOT_FOUND;
        }
    } else {
        // reverse find out by address
        for (auto & it : storage) {
            if (sameSocketAddress(&request.sockaddr, &it.second.sockaddr)) {
                retVal = it.second;
                return CODE_OK;
            }
        }
        return ERR_CODE_GATEWAY_NOT_FOUND;
    }
}

// List entries
int  JsonGatewayService::list(
    std::vector<GatewayIdentity> &retVal,
    uint32_t offset,
    uint8_t size
) {
    size_t o = 0;
    size_t sz = 0;
    for (auto & it : storage) {
        if (o < offset) {
            // skip first
            o++;
            continue;
        }
        sz++;
        if (sz > size)
            break;
        retVal.push_back(it.second);
    }
    return CODE_OK;
}

// Entries count
size_t JsonGatewayService::size()
{
    return storage.size();
}

int JsonGatewayService::put(
    const GatewayIdentity &request
)
{
    storage[request.gatewayId] = request;
    return CODE_OK;
}

int JsonGatewayService::rm(
    const GatewayIdentity &request
)
{
    if (request.gatewayId) {
        // find out by gateway identifier
        auto r = storage.find(request.gatewayId);
        if (r != storage.end()) {
            storage.erase(r);
            return CODE_OK;
        }
    } else {
        // reverse find out by address
        for (auto it(storage.begin()); it != storage.end(); it++) {
            if (sameSocketAddress(&request.sockaddr, &it->second.sockaddr)) {
                storage.erase(it);
                return CODE_OK;
            }
        }
    }
    return ERR_CODE_GATEWAY_NOT_FOUND;
}

bool JsonGatewayService::load()
{
    std::ifstream f(fileName);
    nlohmann::json js;
    try {
        js = nlohmann::json::parse(f);
    } catch (nlohmann::json::exception &exception) {
        std::cerr << exception.what() << std::endl;
        return false;
    }
    if (!js.is_array())
        return false;
    for (auto& e : js) {
        if (!e.contains("gwid") || !e.contains("addr"))
            continue;
        auto jgwid = e["gwid"];
        if (!jgwid.is_string())
            continue;
        auto jaddr = e["addr"];
        if (!jaddr.is_string())
            continue;
        uint64_t gatewayId = string2gatewayId(jgwid);
        GatewayIdentity gi(gatewayId, jaddr);
        storage[gatewayId] = gi;
    }
    f.close();
    return true;
}

bool JsonGatewayService::store()
{
    std::ofstream f(fileName);
    bool isFirst = true;
    f << "[\n";
    for (auto& e : this->storage) {
        if (isFirst)
            isFirst = false;
        else
            f << ",\n";
        f << e.second.toJsonString();
    }
    f << "]\n";
    f.close();
    return true;
}

int JsonGatewayService::init(
    const std::string &option,
    void *data
)
{
    fileName = option;
    load();
    return CODE_OK;
}

void JsonGatewayService::flush()
{
    store();
}

void JsonGatewayService::done()
{
    clear();
}

void JsonGatewayService::setOption(
    int option,
    void *value
)

{
    // nothing to do
}

EXPORT_SHARED_C_FUNC GatewayService* makeGatewayService1()
{
    return new JsonGatewayService;
}
