#if defined(_MSC_VER) || defined(__MINGW32__)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstring>
#include "lorawan/wrapper/connector-identity-serialization.h"

#include "lorawan/storage/serialization/identity-binary-serialization.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"

EXPORT_SHARED_C_FUNC int connectorIdentityVersion()
{
    return 1;
}

EXPORT_SHARED_C_FUNC int binaryIdentityEUIRequest(
    char *retBuf,
    size_t bufSize,
    char aTag,
    uint64_t aEUI,
    int32_t code,
    uint64_t accessCode
)
{
    if (bufSize < SIZE_DEVICE_EUI_REQUEST)
        return ERR_CODE_INSUFFICIENT_MEMORY;
    IdentityEUIRequest r(aTag, DEVEUI(aEUI), code, accessCode);
    return (int) r.serialize(reinterpret_cast<unsigned char *>(retBuf));
}

EXPORT_SHARED_C_FUNC int binaryIdentityAddrRequest(
    char *retBuf,
    size_t bufSize,
    char aTag,
    uint32_t addr,
    int32_t code,
    uint64_t accessCode
)
{
    if (bufSize < SIZE_DEVICE_ADDR_REQUEST)
        return ERR_CODE_INSUFFICIENT_MEMORY;
    IdentityAddrRequest r(aTag, DEVADDR(addr), code, accessCode);
    return (int) r.serialize(reinterpret_cast<unsigned char *>(retBuf));
}

EXPORT_SHARED_C_FUNC int binaryIdentityAssignRequest(
    char *retBuf,
    size_t bufSize,

    char aTag,
    // NETWORKIDENTITY
    uint32_t addr,
    char *activation,   	///< activation type: ABP or OTAA
    char *deviceClass,      ///< A, B, C
    char *devEUI,		    ///< device identifier 8 bytes (ABP device may not store EUI)
    char *nwkSKey,			///< shared session key 16 bytes
    char *appSKey,			///< private key 16 bytes
    char *version,
    // OTAA
    char *appEUI,			   ///< OTAA application identifier
    char *appKey,			   ///< OTAA application private key
    char *nwkKey,              ///< OTAA network key
    uint16_t devNonce,     ///< last device nonce
    uint32_t joinNonce,    ///< last Join nonce
    // added for searching
    char *name,

    int32_t code,
    uint64_t accessCode
)
{
    if (bufSize < SIZE_ASSIGN_REQUEST)
        return ERR_CODE_INSUFFICIENT_MEMORY;
    NETWORKIDENTITY identity;
    identity.value.devaddr = DEVADDR(addr);
    identity.value.devid.id.activation = string2activation(activation);   	// activation type: ABP or OTAA
    identity.value.devid.id.deviceclass = string2deviceclass(deviceClass); // A, B, C
    string2DEVEUI(identity.value.devid.id.devEUI, devEUI);		        // device identifier 8 bytes (ABP device may not store EUI)
    string2KEY(identity.value.devid.id.nwkSKey, nwkSKey); //shared session key 16 bytes
    string2KEY(identity.value.devid.id.appSKey, appSKey); // private key 16 bytes
    identity.value.devid.id.version = string2LORAWAN_VERSION(version);
    // OTAA
    string2DEVEUI(identity.value.devid.id.appEUI, devEUI); // OTAA application identifier
    string2KEY(identity.value.devid.id.appKey, appKey); // OTAA application private key
    string2KEY(identity.value.devid.id.nwkKey, nwkKey); // OTAA network key
    identity.value.devid.id.devNonce = DEVNONCE((uint16_t) devNonce);     // last device nonce
    identity.value.devid.id.joinNonce = JOINNONCE((uint32_t) joinNonce); // last Join nonce
    // added for searching
    string2DEVICENAME(identity.value.devid.id.name, name);

    IdentityAssignRequest r(aTag, identity, code, accessCode);
    return (int) r.serialize(reinterpret_cast<unsigned char *>(retBuf));
}

EXPORT_SHARED_C_FUNC int binaryIdentityOperationRequest(
    char *retBuf,
    size_t bufSize,

    char aTag,

    uint32_t aOffset,
    uint8_t aSize,

    int32_t code,
    uint64_t accessCode
)
{
    if (bufSize < SIZE_OPERATION_REQUEST)
        return ERR_CODE_INSUFFICIENT_MEMORY;
    IdentityOperationRequest r(aTag, aOffset, aSize, code, accessCode);
    return (int) r.serialize(reinterpret_cast<unsigned char *>(retBuf));
}

EXPORT_SHARED_C_FUNC int binaryIdentityGetResponse(
    char *buf,
    size_t size,
    // return NETWORKIDENTITY
    uint32_t &addr,
    char **activation,   	///< activation type: ABP or OTAA
    char **deviceClass,     ///< A, B, C
    char **devEUI,		    ///< device identifier 8 bytes (ABP device may not store EUI)
    char **nwkSKey,			///< shared session key 16 bytes
    char **appSKey,			///< private key 16 bytes
    char **version,
        // OTAA
    char **appEUI,			///< OTAA application identifier
    char **appKey,			///< OTAA application private key
    char **nwkKey,          ///< OTAA network key
    uint16_t &devNonce,     ///< last device nonce
    uint32_t &joinNonce,    ///< last Join nonce
    // added for searching
    char **name
) {
    enum IdentityQueryTag tag = validateIdentityResponse((reinterpret_cast<const unsigned char *>(buf)), size);
    switch (tag) {
        case QUERY_IDENTITY_EUI:   // request gateway identifier(with address) by network address.
        case QUERY_IDENTITY_ADDR:   // request gateway address (with identifier) by identifier.
        {
            IdentityGetResponse gr(reinterpret_cast<const unsigned char *>(buf), size);
            gr.ntoh();
            addr = gr.response.value.devaddr.get();
            devNonce = gr.response.value.devid.id.devNonce.u;
            joinNonce = gr.response.value.devid.id.joinNonce.get();
            if (activation) {
                std::string s = activation2string(gr.response.value.devid.id.activation);    // ABP, OTAA
                memmove(*activation, s.c_str(), s.size());
            }
            if (deviceClass) {
                std::string s = deviceclass2string(gr.response.value.devid.id.deviceclass);  // A, B, C
                memmove(*deviceClass, s.c_str(), s.size());
            }
            if (devEUI) {
                std::string s = DEVEUI2string(gr.response.value.devid.id.devEUI);  // device identifier 8 bytes (ABP device may not store EUI)
                memmove(*devEUI, s.c_str(), s.size());
            }
            if (nwkSKey) {
                std::string s = KEY2string(gr.response.value.devid.id.nwkSKey);  // shared session key 16 bytes
                memmove(*nwkSKey, s.c_str(), s.size());
            }
            if (appSKey) {
                std::string s = KEY2string(gr.response.value.devid.id.appSKey);  // shared session key 16 bytes
                memmove(*appSKey, s.c_str(), s.size());
            }
            if (version) {
                std::string s = LORAWAN_VERSION2string(gr.response.value.devid.id.version);  // e.g. 1.0.0
                memmove(*version, s.c_str(), s.size());
            }
            // OTAA
            if (appEUI) {
                std::string s = DEVEUI2string(gr.response.value.devid.id.appEUI);  // OTAA application identifier
                memmove(*appEUI, s.c_str(), s.size());
            }
            if (appKey) {
                std::string s = KEY2string(gr.response.value.devid.id.appKey);  // OTAA application private key
                memmove(*appKey, s.c_str(), s.size());
            }
            if (nwkKey) {
                std::string s = KEY2string(gr.response.value.devid.id.nwkKey);  // OTAA network key
                memmove(*nwkKey, s.c_str(), s.size());
            }
            // added for searching
            if (name) {
                // strncpy_s(*name, sizeof(gr.response.devid.name.c), gr.response.devid.name.c, sizeof(gr.response.devid.name.c));
                strncpy(*name, gr.response.value.devid.id.name.c, sizeof(gr.response.value.devid.id.name.c));
            }
        }
            break;
        default:
            return ERR_CODE_INVALID_PACKET;
    }
    return CODE_OK;
}

EXPORT_SHARED_C_FUNC int binaryIdentityListResponse(
    char *buf,
    size_t size,

    char **retBuf,
    size_t bufSize
)
{
    enum IdentityQueryTag tag = validateIdentityResponse(reinterpret_cast<const unsigned char *>(buf), size);
    switch (tag) {
        case QUERY_IDENTITY_LIST:   // List entries
        {
            IdentityListResponse gr(reinterpret_cast<const unsigned char *>(buf), size);
            gr.ntoh();
            std::string s = gr.toJsonString();
            auto sz = s.size();
            if (sz < bufSize) {
                memmove(*retBuf, s.c_str(), sz);
                if (sz < bufSize)
                    *retBuf[sz] = '\0';
            }
            return (int) sz;
        }
        default:
            return ERR_CODE_INVALID_PACKET;
    }
}

EXPORT_SHARED_C_FUNC int binaryIdentityOperationResponse(
    char *buf,
    size_t size,

    int32_t &code,
    uint64_t &accessCode,
    uint32_t &offset,
    uint8_t &retSize,
    int32_t &response
)
{
    enum IdentityQueryTag tag = validateIdentityResponse(reinterpret_cast<const unsigned char *>(buf), size);
    switch (tag) {
        case QUERY_IDENTITY_ASSIGN:   // assign (put) gateway address to the gateway by identifier
        case QUERY_IDENTITY_RM:   // Remove entry
        case QUERY_IDENTITY_COUNT:   // count
        case QUERY_IDENTITY_NEXT:   // next
        case QUERY_IDENTITY_FORCE_SAVE:   // force save
        case QUERY_IDENTITY_CLOSE_RESOURCES:   // close resources
        {
            IdentityOperationResponse gr(reinterpret_cast<const unsigned char *>(buf), size);
            gr.ntoh();
            code = gr.code;
            accessCode = gr.accessCode;
            offset = gr.offset;
            retSize = gr.size;
            response = gr.response;
        }
            break;
        default:
            return ERR_CODE_INVALID_PACKET;
    }
    return CODE_OK;
}
