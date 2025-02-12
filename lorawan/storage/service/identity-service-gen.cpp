#include <regex>

#include "lorawan/storage/service/identity-service-gen.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/key128gen.h"
#include "lorawan/storage/serialization/identity-binary-serialization.h"
#include "lorawan/lorawan-key.h"

#ifdef ESP_PLATFORM
#include <iostream>
#include "platform-defs.h"
#endif

#ifdef ENABLE_DEBUG
#include <iostream>
#endif

#define DEFAULT_NETID   0

GenIdentityService::GenIdentityService()
    : maxDevNwkAddr(0), errCode(0)
{

}

GenIdentityService::GenIdentityService(
    const std::string &masterKey
)
    : maxDevNwkAddr(0), errCode(0)
{
    setMasterKey(masterKey);
}

void GenIdentityService::setMasterKey(
    const std::string &masterKey
)
{
    phrase2key((uint8_t *) &key.c, masterKey.c_str(), masterKey.size());
}

GenIdentityService::~GenIdentityService() = default;

void GenIdentityService::clear()
{
    maxDevNwkAddr = 0;
}

/**
 * request device identifier by network address. Return 0 if success, retval = EUI and keys
 * @param retval device identifier
 * @param devaddr network address
 * @return LORA_OK- success
 */
int GenIdentityService::get(
    DEVICEID &retval,
    const DEVADDR &devaddr
)
{
    retval.id.activation = ABP;	///< activation type: ABP or OTAA
    retval.id.deviceclass = CLASS_A;
    euiGen((uint8_t *) &retval.id.devEUI.c, KEY_NUMBER_EUI, (uint8_t *) &key.c, devaddr.u);
    keyGen((uint8_t *) &retval.id.appEUI.c, KEY_NUMBER_EUI, (uint8_t *) &key.c, devaddr.u);

    keyGen((uint8_t *) &retval.id.nwkKey.c, KEY_NUMBER_NWK, (uint8_t *) &key.c, devaddr.u);
    keyGen((uint8_t *) &retval.id.appKey.c, KEY_NUMBER_APP, (uint8_t *) &key.c, devaddr.u);

    retval.id.joinNonce = {};
    retval.id.devNonce = {};

    deriveOptNegFNwkSIntKey(retval.id.nwkSKey, key, retval.id.appEUI, retval.id.joinNonce, retval.id.devNonce);
    deriveOptNegFNwkSIntKey(retval.id.appSKey, key, retval.id.appEUI, retval.id.joinNonce, retval.id.devNonce);

    retval.id.version = { 1, 0, 0 };
    string2DEVICENAME(retval.id.name, DEVADDR2string(devaddr).c_str());
#ifdef ENABLE_DEBUG
        std::cerr << "get " << DEVADDR2string(devaddr)
            << std::endl;
#endif
    return 0;
}

/**
* request network identity(with address) by device EUI. Return 0 if success, retval = EUI and keys
* @param retval network identity(with address)
* @param eui device EUI
* @return LORA_OK- success
*/
int GenIdentityService::getNetworkIdentity(
    NETWORKIDENTITY &retval,
    const DEVEUI &eui
) {
    return ERR_CODE_DEVICE_EUI_NOT_FOUND;
}

void GenIdentityService::gen(
    NETWORKIDENTITY &retVal,
    uint32_t nwkAddr
)
{
    retVal.value.devaddr = DEVADDR(netid, nwkAddr);

    euiGen((uint8_t *) &retVal.value.devid.id.devEUI.c, KEY_NUMBER_EUI, (uint8_t *) &key.c, retVal.value.devaddr.u);
    keyGen((uint8_t *) &retVal.value.devid.id.appEUI.c, KEY_NUMBER_EUI, (uint8_t *) &key.c, retVal.value.devaddr.u);

    keyGen((uint8_t *) &retVal.value.devid.id.nwkKey.c, KEY_NUMBER_NWK, (uint8_t *) &key.c, retVal.value.devaddr.u);
    keyGen((uint8_t *) &retVal.value.devid.id.appKey.c, KEY_NUMBER_APP, (uint8_t *) &key.c, retVal.value.devaddr.u);

    retVal.value.devid.id.joinNonce = {};
    retVal.value.devid.id.devNonce = {};

    deriveOptNegFNwkSIntKey(retVal.value.devid.id.nwkSKey, retVal.value.devid.id.nwkKey, retVal.value.devid.id.appEUI, retVal.value.devid.id.joinNonce, retVal.value.devid.id.devNonce);
    deriveOptNegFNwkSIntKey(retVal.value.devid.id.appSKey, retVal.value.devid.id.appKey, retVal.value.devid.id.appEUI, retVal.value.devid.id.joinNonce, retVal.value.devid.id.devNonce);

    retVal.value.devid.id.version = {1, 0, 0 };
    string2DEVICENAME(retVal.value.devid.id.name, DEVADDR2string(retVal.value.devaddr).c_str());
}

// List entries
int GenIdentityService::list(
    std::vector<NETWORKIDENTITY> &retVal,
    uint32_t offset,
    uint8_t size
) {
    uint32_t a = offset;
    size_t sz = netid.size();
    for (int i = 0; i < size; i++) {
        if (a > sz)
            break;
        NETWORKIDENTITY v;
        gen(v, a);
        retVal.push_back(v);
        a++;
    }
    return CODE_OK;
}

// Entries count
size_t GenIdentityService::size()
{
    return netid.size();
}

int GenIdentityService::put(
    const DEVADDR &devaddr,
    const DEVICEID &id
)
{
    return CODE_OK;
}

int GenIdentityService::rm(
    const DEVADDR &addr
)
{
    return CODE_OK;
}

int GenIdentityService::init(
    const std::string &option,
    void *data
)
{
    setMasterKey(option);
    if (data)
        netid.set(*(NETID*)data);
    else
        netid.set(DEFAULT_NETID);   // set default network id
    return CODE_OK;
}

void GenIdentityService::flush()
{
}

void GenIdentityService::done()
{

}

/**
  * Return next network address if available
  * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
  */
int GenIdentityService::next(
    NETWORKIDENTITY &retval
)
{
    return ERR_CODE_ADDR_SPACE_FULL;
}

/**
  * Return next available network address
  * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
  */
int GenIdentityService::nextBruteForce(
    NETWORKIDENTITY &retVal
)
{
    return ERR_CODE_ADDR_SPACE_FULL;
}

void GenIdentityService::setOption(
    int option,
    void *value
)

{
    if (!value)
        return;
    if (option == 0)
        setMasterKey(*(std::string *) value);
}

// ------------------- asynchronous imitations -------------------

int GenIdentityService::cGet(const DEVADDR &request)
{
    IdentityGetResponse r;
    r.response.value.devaddr = request;
    get(r.response.value.devid, request);
    if (responseClient)
        responseClient->onIdentityGet(nullptr, &r);
    return CODE_OK;
}

int GenIdentityService::cGetNetworkIdentity(const DEVEUI &eui)
{
    IdentityGetResponse r;
    getNetworkIdentity(r.response, eui);
    if (responseClient)
        responseClient->onIdentityGet(nullptr, &r);
    return CODE_OK;
}

int GenIdentityService::cPut(const DEVADDR &devAddr, const DEVICEID &id)
{
    IdentityOperationResponse r;
    r.response = put(devAddr, id);
    if (responseClient)
        responseClient->onIdentityOperation(nullptr, &r);
    return CODE_OK;
}

int GenIdentityService::cRm(const DEVADDR &devAddr)
{
    IdentityOperationResponse r;
    r.response = rm(devAddr);
    if (responseClient)
        responseClient->onIdentityOperation(nullptr, &r);
    return CODE_OK;
}

int GenIdentityService::cList(uint32_t offset, uint8_t size)
{
    IdentityListResponse r;
    r.response = list(r.identities, offset, size);
    r.size = (uint8_t) r.identities.size();
    if (responseClient)
        responseClient->onIdentityList(nullptr, &r);
    return CODE_OK;
}

int GenIdentityService::cSize()
{
    IdentityOperationResponse r;
    r.size = (uint8_t) size();
    if (responseClient)
        responseClient->onIdentityOperation(nullptr, &r);
    return CODE_OK;
}

int GenIdentityService::cNext()
{
    IdentityGetResponse r;
    next(r.response);
    if (responseClient)
        responseClient->onIdentityGet(nullptr, &r);
    return CODE_OK;
}

int GenIdentityService::filter(
    std::vector<NETWORKIDENTITY> &retVal,
    const std::vector<NETWORK_IDENTITY_FILTER> &filters,
    uint32_t offset,
    uint8_t size
)
{
    // logically incorrect to avoid infinite loop
    uint32_t a = offset;
    size_t sz = netid.size();
    for (int i = 0; i < size; i++) {
        if (a > sz)
            break;
        NETWORKIDENTITY v;
        gen(v, a);
        if (!isIdentityFilteredV(v, filters))
            continue;
        retVal.push_back(v);
        a++;
    }
    return CODE_OK;
}

int GenIdentityService::cFilter(
    const std::vector<NETWORK_IDENTITY_FILTER> &filters,
    uint32_t offset,
    uint8_t size
)
{
    return cList(offset, size);
}

EXPORT_SHARED_C_FUNC IdentityService* makeIdentityService()
{
    return new GenIdentityService;
}
