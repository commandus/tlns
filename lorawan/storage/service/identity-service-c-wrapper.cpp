#include <functional>
#include <cstring>
#if defined(_MSC_VER) || defined(__MINGW32__)
#include <WinSock2.h>
#endif
#include "lorawan/storage/service/identity-service-c-wrapper.h"
#include <lorawan/lorawan-string.h>
#include "lorawan/storage/service/identity-service.h"
#include "lorawan/storage/service/identity-service-mem.h"
#include "lorawan/storage/service/identity-service-udp.h"

#ifdef ENABLE_GEN
#include "identity-service-gen.h"
#endif
#ifdef ENABLE_JSON
#include "identity-service-json.h"
#endif
#ifdef ENABLE_SQLITE
#include "identity-service-sqlite.h"
#endif
#ifdef ENABLE_LMDB
#include "identity-service-lmdb.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

EXPORT_SHARED_C_FUNC void *createIdentityServiceC(
        void *instance
) {
    return instance;
}

EXPORT_SHARED_C_FUNC void *makeIdentityServiceC(
    C_IDENTITY_SERVICE_IMPL impl
) {
    switch (impl) {
#ifdef ENABLE_GEN
        case CISI_GEN:
            return makeIdentityService();
#endif
#ifdef ENABLE_JSON
        case CISI_JSON:
            return makeIdentityService1();
#endif
        case CISI_MEM:
            return makeIdentityService2();
#ifdef ENABLE_SQLITE
        case CISI_SQLITE:
            return makeIdentityService3();
#endif
        case CISI_UDP:
            return makeIdentityService4();
#ifdef ENABLE_LMDB
        case CISI_LMDB:
            return makeIdentityService5();
#endif
        default:
            return nullptr;
    }
}

EXPORT_SHARED_C_FUNC void destroyIdentityServiceC(
    void *instance
) {
    if (instance)
        delete (IdentityService *) instance;
}

static void DEVICEID2C_DEVICEID(
    C_DEVICEID *retVal,
    const DEVICEID &did
) {
    retVal->activation = did.id.activation;
    retVal->deviceclass = did.id.deviceclass;
    retVal->devEUI = did.id.devEUI.u;
    memmove(&retVal->nwkSKey, &did.id.nwkSKey, sizeof(KEY128));
    memmove(&retVal->appSKey, &did.id.appSKey, sizeof(KEY128));
    retVal->version = did.id.version.c;
    retVal->appEUI = did.id.appEUI.u;
    memmove(&retVal->appKey, &did.id.appKey, sizeof(KEY128));
    memmove(&retVal->nwkKey, &did.id.nwkKey, sizeof(KEY128));
    retVal->devNonce = did.id.devNonce.u;
    memmove(&retVal->joinNonce, &did.id.joinNonce, sizeof(C_JOINNONCE));
    memmove(&retVal->name, &did.id.name, sizeof(C_DEVICENAME));
}

static void NETWORKIDENTITY2C_NETWORKIDENTITY(
    C_NETWORKIDENTITY *retVal,
    const NETWORKIDENTITY &nid
) {
    retVal->devaddr = nid.value.devaddr.u;
    retVal->devid.activation = nid.value.devid.id.activation;
    retVal->devid.deviceclass = nid.value.devid.id.deviceclass;
    retVal->devid.devEUI = nid.value.devid.id.devEUI.u;
    memmove(&retVal->devid.nwkSKey, &nid.value.devid.id.nwkSKey, sizeof(KEY128));
    memmove(&retVal->devid.appSKey, &nid.value.devid.id.appSKey, sizeof(KEY128));
    retVal->devid.version = nid.value.devid.id.version.c;
    retVal->devid.appEUI = nid.value.devid.id.appEUI.u;
    memmove(&retVal->devid.appKey, &nid.value.devid.id.appKey, sizeof(KEY128));
    memmove(&retVal->devid.nwkKey, &nid.value.devid.id.nwkKey, sizeof(KEY128));
    retVal->devid.devNonce = nid.value.devid.id.devNonce.u;
    memmove(&retVal->devid.joinNonce, &nid.value.devid.id.joinNonce, sizeof(C_JOINNONCE));
    memmove(&retVal->devid.name, &nid.value.devid.id.name, sizeof(C_DEVICENAME));
}

static void C_DEVICEID2DEVICEID(
    DEVICEID &retVal,
    C_DEVICEID *did
) {
    retVal.id.activation = (ACTIVATION) did->activation;
    retVal.id.deviceclass = (DEVICECLASS) did->deviceclass;
    retVal.id.devEUI.u = did->devEUI;
    memmove(&retVal.id.nwkSKey, &did->nwkSKey, sizeof(KEY128));
    memmove(&retVal.id.appSKey, &did->appSKey, sizeof(KEY128));
    retVal.id.version.c = did->version;
    retVal.id.appEUI.u = did->appEUI;
    memmove(&retVal.id.appKey, &did->appKey, sizeof(KEY128));
    memmove(&retVal.id.nwkKey, &did->nwkKey, sizeof(KEY128));
    retVal.id.devNonce.u = did->devNonce;
    memmove(&retVal.id.joinNonce, &did->joinNonce, sizeof(C_JOINNONCE));
    memmove(&retVal.id.name, &did->name, sizeof(C_DEVICENAME));
}

static void C_NETWORKIDENTITY2NETWORKIDENTITY(
    NETWORKIDENTITY &retVal,
    C_NETWORKIDENTITY *nid
) {
    retVal.value.devaddr.u = nid->devaddr;
    C_DEVICEID2DEVICEID(retVal.value.devid, &nid->devid);
}

static void C_NETWORK_IDENTITY_FILTER2NETWORK_IDENTITY_FILTER(
    NETWORK_IDENTITY_FILTER &retVal,
    C_NETWORK_IDENTITY_FILTER *filter
) {
    retVal.pre = (NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR) filter->pre;
    retVal.property = (NETWORK_IDENTITY_PROPERTY) filter->property;
    retVal.comparisonOperator = (NETWORK_IDENTITY_COMPARISON_OPERATOR) filter->comparisonOperator;
    retVal.length = filter->length;
    memmove(retVal.filterData, filter->filterData, sizeof(retVal.filterData));
}

static void JOIN_ACCEPT_FRAME_HEADER2C_JOIN_ACCEPT_FRAME_HEADER(
        C_JOIN_ACCEPT_FRAME_HEADER *retVal,
        const JOIN_ACCEPT_FRAME_HEADER &hdr
) {
    retVal->joinNonce[0] = hdr.joinNonce.c[0];
    retVal->joinNonce[1] = hdr.joinNonce.c[1];
    retVal->joinNonce[2] = hdr.joinNonce.c[2];
    retVal->netId[0] = hdr.netId.c[0];
    retVal->netId[1] = hdr.netId.c[1];
    retVal->netId[2] = hdr.netId.c[2];
    retVal->devAddr = hdr.devAddr.u;
    retVal->dlSettings.c = hdr.dlSettings.c;            // downlink configuration settings
    retVal->rxDelay = hdr.rxDelay;
}

EXPORT_SHARED_C_FUNC int c_get(
    void *o,
    C_DEVICEID *retVal,
    const C_DEVADDR *devAddr
) {
    const DEVADDR a(*devAddr);
    DEVICEID did;
    int r = ((IdentityService *) o)->get(did, a);
    DEVICEID2C_DEVICEID(retVal, did);
    return r;
}

EXPORT_SHARED_C_FUNC int c_getNetworkIdentity(
    void *o,
    C_NETWORKIDENTITY *retVal,
    const C_DEVEUI *eui
) {
    NETWORKIDENTITY nid;
    const DEVEUI devEui(*eui);
    int r = ((IdentityService *) o)->getNetworkIdentity(nid, devEui);
    NETWORKIDENTITY2C_NETWORKIDENTITY(retVal, nid);
    return r;
}

EXPORT_SHARED_C_FUNC int c_put(
    void *o,
    const C_DEVADDR *devaddr,
    const C_DEVICEID *id
) {
    const DEVADDR a(*devaddr);
    const DEVICEID did(
            static_cast<ACTIVATION>(id->activation),
            static_cast<DEVICECLASS>(id->deviceclass),
            static_cast<DEVEUI>(id->devEUI),
            *(KEY128 *) &id->nwkSKey,
            *(KEY128 *) &id->appSKey,
            id->version,
            static_cast<DEVEUI>(id->appEUI),
            *(KEY128 *) &id->appKey,
            *(KEY128 *) &id->nwkKey,
            static_cast<DEVNONCE>(id->devNonce),
            *(JOINNONCE *) &id->joinNonce,
            static_cast<DEVICENAME>(id->name)
    );
    return ((IdentityService *) o)->put(a, did);
}

EXPORT_SHARED_C_FUNC int c_rm(
    void *o,
    const C_DEVADDR *addr
) {
    const DEVADDR a(*addr);
    return ((IdentityService *) o)->rm(a);
}

EXPORT_SHARED_C_FUNC int c_list(
    void *o,
    C_NETWORKIDENTITY retVal[],
    uint32_t offset,
    uint8_t size
) {
    std::vector<NETWORKIDENTITY> v;
    int r = ((IdentityService *) o)->list(v, offset, size);
    if (r >= 0) {
        for (auto i = 0; i < v.size(); i++) {
            retVal[i].devaddr = v[i].value.devaddr.u;
            NETWORKIDENTITY2C_NETWORKIDENTITY(&retVal[i], v[i]);
        }
    }
    return r < 0 ? r : (int) v.size();
}

EXPORT_SHARED_C_FUNC int c_filter(
    void *o,
    C_NETWORKIDENTITY retVal[],
    C_NETWORK_IDENTITY_FILTER filters[],
    size_t filterSize,
    uint32_t offset,
    uint8_t size
) {
    std::vector<NETWORKIDENTITY> v;
    std::vector<NETWORK_IDENTITY_FILTER> f;
    for (auto i = 0; i < filterSize; i++) {
        NETWORK_IDENTITY_FILTER nif;
        C_NETWORK_IDENTITY_FILTER2NETWORK_IDENTITY_FILTER(nif, &filters[i]);
        f.emplace_back(nif);
    }
    int r = ((IdentityService *) o)->filter(v, f, offset, size);
    for (auto i = 0; i < v.size(); i++) {
        retVal[i].devaddr = v[i].value.devaddr.u;
        NETWORKIDENTITY2C_NETWORKIDENTITY(&retVal[i], v[i]);

    }
    return r < 0 ? r : (int) v.size();
}

EXPORT_SHARED_C_FUNC int c_filterExpression(
    void *o,
    C_NETWORKIDENTITY retVal[],
    const char *filterExpression,
    size_t filterExpressionSize,
    uint32_t offset,
    uint8_t size
) {
    std::vector<NETWORKIDENTITY> v;
    std::vector<NETWORK_IDENTITY_FILTER> f;

    string2NETWORK_IDENTITY_FILTERS(f, filterExpression, filterExpressionSize);
    int r = ((IdentityService *) o)->filter(v, f, offset, size);
    for (auto i = 0; i < v.size(); i++) {
        retVal[i].devaddr = v[i].value.devaddr.u;
        NETWORKIDENTITY2C_NETWORKIDENTITY(&retVal[i], v[i]);
    }
    return r < 0 ? r : (int) v.size();
}

EXPORT_SHARED_C_FUNC size_t c_size(
    void *o
) {
    return ((IdentityService *) o)->size();
}

EXPORT_SHARED_C_FUNC int c_next(
    void *o,
    C_NETWORKIDENTITY *retVal
) {
    NETWORKIDENTITY nid;
    int r = ((IdentityService *) o)->next(nid);
    NETWORKIDENTITY2C_NETWORKIDENTITY(retVal, nid);
    return r;
}

EXPORT_SHARED_C_FUNC void c_flush(
    void *o
) {
    ((IdentityService *) o)->flush();
}

EXPORT_SHARED_C_FUNC int c_init(
    void *o,
    const char *option,
    void *data
) {
    return ((IdentityService *) o)->init(option, data);
}

EXPORT_SHARED_C_FUNC void c_done(
    void *o
) {
    ((IdentityService *) o)->flush();
    ((IdentityService *) o)->done();
}

EXPORT_SHARED_C_FUNC void c_setOption(
    void *o,
    int option,
    void *value
) {
    ((IdentityService *) o)->setOption(option, value);
}

EXPORT_SHARED_C_FUNC C_NETID *c_getNetworkId(
        void *o
) {
    return (C_NETID *) &((IdentityService *) o)->getNetworkId()->c;
}

EXPORT_SHARED_C_FUNC void c_setNetworkId(
        void *o,
        const C_NETID *value
) {
    NETID n;
    n.c[0] = *value[0];
    n.c[1] = *value[1];
    n.c[2] = *value[2];
    ((IdentityService *) o)->setNetworkId(n);
}

EXPORT_SHARED_C_FUNC int c_joinAccept(
    void *o,
    C_JOIN_ACCEPT_FRAME_HEADER *retVal,
    C_NETWORKIDENTITY *networkIdentity
) {
    NETWORKIDENTITY nid;
    C_NETWORKIDENTITY2NETWORKIDENTITY(nid, networkIdentity);
    JOIN_ACCEPT_FRAME_HEADER path;
    int r = ((IdentityService *) o)->joinAccept(path, nid);
    JOIN_ACCEPT_FRAME_HEADER2C_JOIN_ACCEPT_FRAME_HEADER(retVal, path);
    return r;
}

EXPORT_SHARED_C_FUNC void text2c_devaddr(
    C_DEVADDR *retVal,
    const char *expr
)
{
    DEVADDR a;
    string2DEVADDR(a, expr);
    *retVal = a.u;
}

EXPORT_SHARED_C_FUNC void text2c_deviceid(
    C_DEVICEID *retVal,
    const char *lines[]
)
{
    if (!retVal || !lines)
        return;
    DEVICEID did;
    // lines[0] - address
    did.id.activation = string2activation(lines[1]);
    did.id.deviceclass = string2deviceclass(lines[2]);
    string2DEVEUI(did.id.devEUI, lines[3]);
    string2KEY(did.id.nwkSKey, lines[4]);
    string2KEY(did.id.appSKey, lines[5]);
    did.id.version = string2LORAWAN_VERSION(lines[6]);
    string2DEVEUI(did.id.appEUI, lines[7]);
    string2KEY(did.id.appKey, lines[8]);
    string2KEY(did.id.nwkKey, lines[9]);
    did.id.devNonce = string2DEVNONCE(lines[10]);
    string2JOINNONCE(did.id.joinNonce, lines[11]);
    string2DEVICENAME(did.id.name, lines[12]);

    DEVICEID2C_DEVICEID(retVal, did);
}

EXPORT_SHARED_C_FUNC void text2c_networkidentity(
    C_NETWORKIDENTITY *retval,
    const char *lines[]
) {
    C_DEVADDR a;
    text2c_devaddr(&a, lines[0]);
    retval->devaddr = a;
    text2c_deviceid(&retval->devid, lines);
}

static size_t addString2Buffer(
    char *buffer,
    size_t position,
    size_t bufferSize,
    int index,
    char *retVal[],
    const std::string &s
)
{
    size_t len = s.size() + 1;
    if (position + len <= bufferSize) {
        memmove(buffer + position, s.c_str(), len);
        retVal[index] = buffer + position;
    } else
        retVal[index] = nullptr;
    return len;
}

EXPORT_SHARED_C_FUNC size_t c_devaddr2text(
    char *buffer,
    size_t bufferSize,
    C_DEVADDR *addr
)
{
    DEVADDR a(*addr);
    std::string r(DEVADDR2string(a));
    auto sz = r.size();
    if (sz <= bufferSize)
        memmove(buffer, r.c_str(), sz);
    return sz;
}

EXPORT_SHARED_C_FUNC void c_deviceid2text(
    char *buffer,
    size_t bufferSize,
    char *retVal[],
    C_DEVICEID *deviceId
)
{
    if (!buffer || !bufferSize || !retVal)
        return;
    DEVICEID did;
    C_DEVICEID2DEVICEID(did, deviceId);
    size_t position = 0;
    position += addString2Buffer(buffer, position, bufferSize, 1, retVal, activation2string(did.id.activation));
    position += addString2Buffer(buffer, position, bufferSize, 2, retVal, deviceclass2string(did.id.deviceclass));
    position += addString2Buffer(buffer, position, bufferSize, 3, retVal, DEVEUI2string(did.id.devEUI));
    position += addString2Buffer(buffer, position, bufferSize, 4, retVal, KEY2string(did.id.nwkSKey));
    position += addString2Buffer(buffer, position, bufferSize, 5, retVal, KEY2string(did.id.appSKey));
    position += addString2Buffer(buffer, position, bufferSize, 6, retVal, LORAWAN_VERSION2string(did.id.version));
    position += addString2Buffer(buffer, position, bufferSize, 7, retVal, DEVEUI2string(did.id.appEUI));
    position += addString2Buffer(buffer, position, bufferSize, 8, retVal, KEY2string(did.id.nwkKey));
    position += addString2Buffer(buffer, position, bufferSize, 9, retVal, KEY2string(did.id.appKey));
    position += addString2Buffer(buffer, position, bufferSize, 10, retVal, DEVNONCE2string(did.id.devNonce));
    position += addString2Buffer(buffer, position, bufferSize, 11, retVal, JOINNONCE2string(did.id.joinNonce));
    position += addString2Buffer(buffer, position, bufferSize, 12, retVal, DEVICENAME2string(did.id.name));
}

EXPORT_SHARED_C_FUNC void c_networkidentity2text(
    char *buffer,
    size_t bufferSize,
    char *retVal[],
    C_NETWORKIDENTITY *networkIdentity
)
{
    if (!buffer || !bufferSize || !retVal)
        return;
    C_DEVADDR a = networkIdentity->devaddr;
    size_t position = c_devaddr2text(buffer, bufferSize, &a);
    if (position >= bufferSize)
        return;
    // add string terminator
    buffer[position] = '\0';
    retVal[0] = buffer;
    position++;
    c_deviceid2text(buffer + position, bufferSize - position, retVal, &networkIdentity->devid);
}

#ifdef __cplusplus
}
#endif
