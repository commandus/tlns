#ifndef GATEWAY_SERIALIZATION_H
#define GATEWAY_SERIALIZATION_H

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <cinttypes>
#endif
#include "lorawan/storage/service/gateway-service.h"
#include "lorawan/storage/serialization/serialization.h"
#include "lorawan/storage/serialization/service-serialization.h"

class GatewaySerialization : public Serialization {
public:
    GatewayService *svc;
    int32_t code;
    uint64_t accessCode;

    explicit GatewaySerialization(
        SerializationKnownType serializationKnownType,
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
    virtual size_t query(
        unsigned char *retBuf,
        size_t retSize,
        const unsigned char *request,
        size_t sz
    ) = 0;
};

#endif
