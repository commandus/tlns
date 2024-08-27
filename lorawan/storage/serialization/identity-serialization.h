#ifndef IDENTITY_SERIALIZATION_H
#define IDENTITY_SERIALIZATION_H

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <cinttypes>
#endif
#include "lorawan/storage/serialization/serialization.h"
#include "lorawan/storage/service/identity-service.h"

class IdentitySerialization : public Serialization {
public:
    int32_t code;
    uint64_t accessCode;
    IdentityService *svc;
    explicit IdentitySerialization(
        SerializationKnownType typ,
        IdentityService *svc,
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
    virtual size_t query(
        unsigned char *retBuf,
        size_t retSize,
        const unsigned char *request,
        size_t sz
    ) = 0;
};

#endif
