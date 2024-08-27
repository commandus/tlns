#include <sstream>
#include <iostream>
#include <fstream>
#include "lorawan/storage/service/identity-service-json.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/file-helper.h"

#ifdef ESP_PLATFORM
#include <iostream>
#include "platform-defs.h"
#endif

JsonIdentityService::JsonIdentityService() = default;

JsonIdentityService::~JsonIdentityService() = default;

/**
 * request device identifier by network address. Return 0 if success, retval = EUI and keys
 * @param retval device identifier
 * @param devaddr network address
 * @return CODE_OK- success
 */
int JsonIdentityService::get(
    DEVICEID &retVal,
    const DEVADDR &request
)
{
    auto r = storage.find(request);
    if (r != storage.end())
        retVal = r->second;
    else
        return ERR_CODE_GATEWAY_NOT_FOUND;
    return CODE_OK;
}

// List entries
int JsonIdentityService::list(
    std::vector<NETWORKIDENTITY> &retVal,
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
        retVal.emplace_back(it.first, it.second);
    }
    return CODE_OK;
}

// Entries count
size_t JsonIdentityService::size()
{
    return storage.size();
}

/**
* request network identity(with address) by network address. Return 0 if success, retval = EUI and keys
* @param retval network identity(with address)
* @param eui device EUI
* @return CODE_OK- success
*/
int JsonIdentityService::getNetworkIdentity(
    NETWORKIDENTITY &retVal,
    const DEVEUI &eui
)
{
    for(auto & it : storage) {
        if (it.second.devEUI.u == eui.u) {
            retVal.devaddr = it.first;
            retVal.devid = it.second;
            return CODE_OK;
        }
    }
    return ERR_CODE_GATEWAY_NOT_FOUND;
}

/**
 * UPSERT SQLite >= 3.24.0
 * @param request gateway identifier or address
 * @return 0- success
 */
int JsonIdentityService::put(
    const DEVADDR &devAddr,
    const DEVICEID &id
)
{
    storage[devAddr] = id;
    return CODE_OK;
}

int JsonIdentityService::rm(
    const DEVADDR &addr
)
{
    // find out by gateway identifier
    auto r = storage.find(addr);
    if (r != storage.end()) {
        storage.erase(r);
        return CODE_OK;
    }
    return ERR_CODE_DEVICE_ADDRESS_NOTFOUND;
}

bool JsonIdentityService::load()
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
        DEVADDR a;
        if (!e.contains("addr"))
            continue;
        string2DEVADDR(a, e["addr"]);
        DEVICEID id;
        if (e.contains("activation"))
            id.activation = string2activation(e["activation"]);
        if (e.contains("class"))
            id.setClass(string2deviceclass(e["class"]));
        if (e.contains("deveui"))
            string2DEVEUI(id.devEUI, e["deveui"]);
        if (e.contains("nwkSKey"))
            string2KEY(id.nwkSKey, e["nwkSKey"]);
        if (e.contains("appSKey"))
            string2KEY(id.appSKey, e["appSKey"]);
        if (e.contains("version"))
            id.version = string2LORAWAN_VERSION(e["version"]);
        if (e.contains("appeui"))
            string2DEVEUI(id.appEUI, e["appeui"]);
        if (e.contains("appKey"))
            string2KEY(id.appKey, e["appKey"]);
        if (e.contains("nwkKey"))
            string2KEY(id.nwkKey, e["nwkKey"]);
        if (e.contains("devNonce"))
            id.devNonce = string2DEVNONCE(e["devNonce"]);
        if (e.contains("joinNonce"))
            string2JOINNONCE(id.joinNonce, e["joinNonce"]);
        if (e.contains("name")) {
            std::string s = e["name"];
            string2DEVICENAME(id.name, s.c_str());
        }
        storage[a] = id;
    }
    f.close();
    return true;
}

bool JsonIdentityService::store()
{
    std::ofstream f(fileName);
    bool isFirst = true;
    f << "[\n";
    for (auto& e : this->storage) {
        if (isFirst)
            isFirst = false;
        else
            f << ",\n";
        f << e.second.toJsonString(e.first);
    }
    f << "]\n";
    f.close();
    return true;
}

int JsonIdentityService::init(
    const std::string &databaseName,
    void *database
)
{
    fileName = databaseName;
    return load() ? CODE_OK : ERR_CODE_INVALID_JSON;
}

void JsonIdentityService::flush()
{
    store();
}

void JsonIdentityService::done()
{
    storage.clear();
}

/**
 * Return next network address if available
 * @return 0- success, ERR_CODE_ADDR_SPACE_FULL- no address available
 */
int JsonIdentityService::next(
    NETWORKIDENTITY &retval
)
{
    return ERR_CODE_ADDR_SPACE_FULL;
}

void JsonIdentityService::setOption(
    int option,
    void *value
)

{
    // nothing to do
}

EXPORT_SHARED_C_FUNC IdentityService* makeIdentityService1()
{
    return new JsonIdentityService;
}
