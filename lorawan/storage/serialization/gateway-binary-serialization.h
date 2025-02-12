#ifndef GATEWAY_BINARY_SERIALIZATION_H
#define GATEWAY_BINARY_SERIALIZATION_H

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <cinttypes>
#endif
#include "lorawan/storage/service/gateway-service.h"
#include "lorawan/storage/gateway-identity.h"
#include "lorawan/storage/serialization/gateway-serialization.h"
#include "lorawan/storage/serialization/service-serialization.h"

enum GatewayQueryTag {
    QUERY_GATEWAY_NONE = '\0',
    QUERY_GATEWAY_ADDR = 'A',
    QUERY_GATEWAY_ID = 'I',
    QUERY_GATEWAY_LIST = 'L',
    QUERY_GATEWAY_COUNT = 'C',
    QUERY_GATEWAY_ASSIGN = 'P',
    QUERY_GATEWAY_RM = 'R',
    QUERY_GATEWAY_FORCE_SAVE = 'S',
    QUERY_GATEWAY_CLOSE_RESOURCES = 'E'
};

class GatewayIdRequest : public ServiceMessage {
public:
    uint64_t id;
    GatewayIdRequest();
    GatewayIdRequest(char aTag, uint64_t id, int32_t code, uint64_t accessCode);
    GatewayIdRequest(const unsigned char *buf, size_t sz);
    ~GatewayIdRequest() override = default;
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
};

class GatewayAddrRequest : public ServiceMessage {
public:
    sockaddr addr;
    GatewayAddrRequest();
    GatewayAddrRequest(const struct sockaddr &addr, int32_t code, uint64_t accessCode);
    GatewayAddrRequest(const unsigned char *buf, size_t sz);
    ~GatewayAddrRequest() override = default;
    void ntoh() override;
    size_t serializedSize() const;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
};

class GatewayIdAddrRequest : public ServiceMessage {
public:
    GatewayIdentity identity;
    GatewayIdAddrRequest();
    // explicit GatewayIdAddrRequest(const GatewayIdentity &identity);
    GatewayIdAddrRequest(char aTag, const GatewayIdentity &identity, int32_t code, uint64_t accessCode);
    GatewayIdAddrRequest(const unsigned char *buf, size_t sz);
    ~GatewayIdAddrRequest() override = default;
    void ntoh() override;
    size_t serializedSize() const;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
};

class GatewayOperationRequest : public ServiceMessage {
public:
    uint32_t offset;
    uint8_t size;
    GatewayOperationRequest();
    GatewayOperationRequest(char tag, size_t aOffset, size_t aSize, int32_t code, uint64_t accessCode);
    GatewayOperationRequest(const unsigned char *buf, size_t sz);
    ~GatewayOperationRequest() override = default;
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
};

class GatewayGetResponse : public ServiceMessage {
public:
    GatewayIdentity response;

    GatewayGetResponse() = default;
    explicit GatewayGetResponse(const GatewayAddrRequest& request);
    explicit GatewayGetResponse(const GatewayIdRequest &request);
    GatewayGetResponse(const unsigned char *buf, size_t sz);
    ~GatewayGetResponse() override = default;
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    size_t serializedSize() const;
    std::string toJsonString() const override;
};

class GatewayOperationResponse : public GatewayOperationRequest {
public:
    uint32_t response;
    GatewayOperationResponse();
    GatewayOperationResponse(const GatewayOperationResponse& resp);
    GatewayOperationResponse(const unsigned char *buf, size_t sz);
    ~GatewayOperationResponse() override = default;
    explicit GatewayOperationResponse(const GatewayIdAddrRequest &request);
    explicit GatewayOperationResponse(const GatewayOperationRequest &request);
    void ntoh() override;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
};

class GatewayListResponse : public GatewayOperationResponse {
public:
    std::vector<GatewayIdentity> identities;
    GatewayListResponse();
    GatewayListResponse(const GatewayListResponse& resp);
    GatewayListResponse(const unsigned char *buf, size_t sz);
    explicit GatewayListResponse(const GatewayOperationRequest &request);
    ~GatewayListResponse() override = default;
    void ntoh() override;
    size_t serializedSize() const;
    size_t serialize(unsigned char *retBuf) const override;
    std::string toJsonString() const override;
    size_t shortenList2Fit(size_t serializedSize);
};

class GatewayBinarySerialization : public GatewaySerialization {
public:
    explicit GatewayBinarySerialization(
        GatewayService *svc,
        int32_t code,
        uint64_t accessCode
    );
    /**
     * Request GatewayService and return serializred response.
     * @param retBuf buffer to return serialized response
     * @param retSize buffer size
     * @param request serialized request
     * @param sz serialized request size
     * @return GatewayService response size
     */
    size_t query(
        unsigned char *retBuf,
        size_t retSize,
        const unsigned char *request,
        size_t sz
    ) override;
};

/**
 * Return request object or  NULL if packet is invalid
 * @param buf buffer
 * @param sz buffer size
 * @return return NULL if packet is invalid
 */
ServiceMessage* deserializeGateway(
    const unsigned char *buf,
    size_t sz
);

/**
 * Check does it gateway tag in the buffer
 * @param buffer buffer to check
 * @param size buffer size
 * @return true
 */
bool isGatewayTag(
    const unsigned char *buffer,
    size_t size
);

/**
 * Check does it serialized query in the buffer
 * @param buffer buffer to check
 * @param size buffer size
 * @return query tag
 */
enum GatewayQueryTag validateGatewayQuery(
    const unsigned char *buffer,
    size_t size
);

/**
 * Return required size for response
 * @param buffer serialized request
 * @param size buffer size
 * @return size in bytes
 */
size_t responseSizeForGatewayRequest(
    const unsigned char *buffer,
    size_t size
);

const char* gatewayTag2string(enum GatewayQueryTag value);

const std::string &gatewayCommandSet();

GatewayQueryTag isGatewayTag(const char *tag);

#endif
