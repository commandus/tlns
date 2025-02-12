#ifndef IDENTITY_TEXT_URN_SERIALIZATION_H
#define IDENTITY_TEXT_URN_SERIALIZATION_H

/**
 * TR005 LoRaWAN Device Identification QR Codes
 * @see https://resources.lora-alliance.org/document/tr005-lorawan-device-identification-qr-codes
 */
#include <cinttypes>
#include "lorawan/storage/service/identity-service.h"
#include "lorawan/storage/serialization/identity-serialization.h"
#include "lorawan/storage/serialization/service-serialization.h"

class IdentityTextURNSerialization : public IdentitySerialization {
public:
    explicit IdentityTextURNSerialization(
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

#endif
