#include "lorawan/storage/serialization/gateway-serialization.h"

GatewaySerialization::GatewaySerialization(
    SerializationKnownType serializationKnownType,
    GatewayService *aSvc,
    int32_t aCode,
    uint64_t aAccessCode
)
    : Serialization(serializationKnownType), svc(aSvc), code(aCode), accessCode(aAccessCode)
{

}
