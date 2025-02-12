#ifndef TLNS_CONNECTOR_IDENTITY_SERIALIZATION_H
#define TLNS_CONNECTOR_IDENTITY_SERIALIZATION_H

#include "lorawan/helper/plugin-helper.h"
#include <cinttypes>

EXPORT_SHARED_C_FUNC int connectorIdentityVersion();

// Requests

EXPORT_SHARED_C_FUNC int binaryIdentityEUIRequest(
    char *retBuf,
    size_t bufSize,
    char aTag,
    uint64_t aEUI,
    int32_t code,
    uint64_t accessCode
);

EXPORT_SHARED_C_FUNC int binaryIdentityAddrRequest(
    char *retBuf,
    size_t bufSize,
    char aTag,
    uint32_t addr,
    int32_t code,
    uint64_t accessCode
);

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
    uint16_t devNonce,         ///< last device nonce
    uint32_t joinNonce,    ///< last Join nonce
    // added for searching
    char *name,

    int32_t code,
    uint64_t accessCode
);

EXPORT_SHARED_C_FUNC int binaryIdentityOperationRequest(
    char *retBuf,
    size_t bufSize,

    char aTag,

    uint32_t aOffset,
    uint8_t aSize,

    int32_t code,
    uint64_t accessCode
);

// Responses

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
);

EXPORT_SHARED_C_FUNC int binaryIdentityOperationResponse(
    char *buf,
    size_t size,

    int32_t &code,
    uint64_t &accessCode,
    uint32_t &offset,
    uint8_t &retSize,
    int32_t &response
);

EXPORT_SHARED_C_FUNC int binaryIdentityListResponse(
    char *buf,
    size_t size,

    char **retBuf,
    size_t bufSize
);

#endif
