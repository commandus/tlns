#ifndef IDENTITY_BINARY_SERIALIZATION_H
#define IDENTITY_BINARY_SERIALIZATION_H

#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <cinttypes>
#endif
#include "lorawan/storage/service/identity-service.h"
#include "lorawan/storage/serialization/identity-serialization.h"
#include "lorawan/storage/serialization/service-serialization.h"

enum IdentityQueryTag {
    QUERY_IDENTITY_NONE = '\0',
    QUERY_IDENTITY_ADDR = 'a',
    QUERY_IDENTITY_EUI = 'i',
    QUERY_IDENTITY_LIST = 'l',
    QUERY_IDENTITY_COUNT = 'c',
    QUERY_IDENTITY_NEXT = 'n',
    QUERY_IDENTITY_ASSIGN = 'p',
    QUERY_IDENTITY_RM = 'r',
    QUERY_IDENTITY_FORCE_SAVE = 's',
    QUERY_IDENTITY_CLOSE_RESOURCES = 'e'
};

// 13 + 4 + 1
#define SIZE_OPERATION_REQUEST 18
#define SIZE_OPERATION_RESPONSE 22
#define SIZE_DEVICE_EUI_REQUEST  21
#define SIZE_DEVICE_ADDR_REQUEST 17
#define SIZE_DEVICE_EUI_ADDR_REQUEST 25
#define SIZE_NETWORK_IDENTITY 141
#define SIZE_ASSIGN_REQUEST 154
#define SIZE_GET_RESPONSE 154

class IdentityEUIRequest : public ServiceMessage {
public:
    DEVEUI eui; // 8 bytes
    IdentityEUIRequest();
    IdentityEUIRequest(char aTag, const DEVEUI &aEUI, int32_t code, uint64_t accessCode);
    IdentityEUIRequest(const unsigned char *buf, size_t sz);
    ~IdentityEUIRequest() override = default;
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
};

class IdentityAddrRequest : public ServiceMessage {
public:
    DEVADDR addr;   // 4 bytes
    IdentityAddrRequest();
    IdentityAddrRequest(char aTag, const DEVADDR &addr, int32_t code, uint64_t accessCode);
    IdentityAddrRequest(const unsigned char *buf, size_t sz);
    ~IdentityAddrRequest() override = default;
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
};

class IdentityAssignRequest : public ServiceMessage {
public:
    NETWORKIDENTITY identity;
    IdentityAssignRequest();
    // explicit IdentityAssignRequest(const DeviceIdentity &identity);
    IdentityAssignRequest(char aTag, const NETWORKIDENTITY &identity, int32_t code, uint64_t accessCode);
    IdentityAssignRequest(const unsigned char *buf, size_t sz);
    ~IdentityAssignRequest() override = default;
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
};

class IdentityOperationRequest : public ServiceMessage {
public:
    uint32_t offset;
    uint8_t size;
    IdentityOperationRequest();
    IdentityOperationRequest(char tag, uint32_t aOffset, uint8_t aSize, int32_t code, uint64_t accessCode);
    IdentityOperationRequest(const unsigned char *buf, size_t sz);
    ~IdentityOperationRequest() override = default;
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
};

class IdentityGetResponse : public ServiceMessage {
public:
    NETWORKIDENTITY response;

    IdentityGetResponse() = default;
    explicit IdentityGetResponse(const IdentityAddrRequest& request);
    explicit IdentityGetResponse(const IdentityEUIRequest &request);
    IdentityGetResponse(const unsigned char *buf, size_t sz);
    ~IdentityGetResponse() override = default;
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
};

class IdentityNextResponse : public ServiceMessage {
public:
    NETWORKIDENTITY response;

    IdentityNextResponse() = default;
    IdentityNextResponse(const unsigned char *buf, size_t sz);
    ~IdentityNextResponse() override = default;
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
};

class IdentityOperationResponse : public IdentityOperationRequest {
public:
    int32_t response;   // <0 - error
    IdentityOperationResponse();
    IdentityOperationResponse(const IdentityOperationResponse& resp);
    IdentityOperationResponse(const unsigned char *buf, size_t sz);
    ~IdentityOperationResponse() override = default;
    explicit IdentityOperationResponse(const IdentityAssignRequest &request);
    explicit IdentityOperationResponse(const IdentityAddrRequest &request);
    explicit IdentityOperationResponse(const IdentityOperationRequest &request);
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
};

class IdentityListResponse : public IdentityOperationResponse {
public:
    std::vector<NETWORKIDENTITY> identities;
    IdentityListResponse();
    IdentityListResponse(const IdentityListResponse& resp);
    IdentityListResponse(const unsigned char *buf, size_t sz);
    explicit IdentityListResponse(const IdentityOperationRequest &request);
    ~IdentityListResponse() override = default;
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
    size_t shortenList2Fit(size_t serializedSize);
};

/**
 * Return request object or NULL if packet is invalid
 * @param buf buffer
 * @param sz buffer size
 * @return return NULL if packet is invalid
 */
ServiceMessage* deserializeIdentity(
    const unsigned char *buf,
    size_t sz
);

/**
 * Check does it identity tag in the buffer
 * @param buffer buffer to check
 * @param size buffer size
 * @return true
 */
bool isIdentityTag(
    const unsigned char *buffer,
    size_t size
);

/**
 * Check does it serialized query in the buffer
 * @param buffer buffer to check
 * @param size buffer size
 * @return query tag
 */
enum IdentityQueryTag validateIdentityQuery(
    const unsigned char *buffer,
    size_t size
);

/**
 * Check does it serialized response in the buffer
 * @param buffer buffer to check
 * @param size buffer size
 * @return query tag
 */
enum IdentityQueryTag validateIdentityResponse(
    const unsigned char *buffer,
    size_t size
);

/**
 * Return required size for response
 * @param buffer serialized request
 * @param size buffer size
 * @return size in bytes
 */
size_t responseSizeForIdentityRequest(
    const unsigned char *buffer,
    size_t size
);

const char* identityTag2string(
    enum IdentityQueryTag value
);

const std::string &identityCommandSet();

class IdentityBinarySerialization : public IdentitySerialization {
public:
    explicit IdentityBinarySerialization(
        IdentityService* svc,
        int32_t code,
        uint64_t accessCode
    );
    /**
     * Request IdentityService and return serialized response.
     * @param retBuf buffer to return serialized response
     * @param retSize buffer size
     * @param request serialized request
     * @param sz serialized request size
     * @return IdentityService response size
     */
    size_t query(
        unsigned char* retBuf,
        size_t retSize,
        const unsigned char* request,
        size_t sz
    ) override;
};

IdentityQueryTag isIdentityTag(const char *tag);

#endif
