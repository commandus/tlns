#include "identity-serialization.h"

IdentitySerialization::IdentitySerialization(
    SerializationKnownType typ,
    IdentityService* aSvc,
    int32_t aCode,
    uint64_t aAccessCode
)
    : Serialization(typ), code(aCode), accessCode(aAccessCode), svc(aSvc)
{

}
