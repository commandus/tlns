#ifndef GATEWAY_TEXT_JSON_SERIALIZATION_H
#define GATEWAY_TEXT_JSON_SERIALIZATION_H

#include <cinttypes>
#include "lorawan/storage/service/gateway-service.h"
#include "lorawan/storage/gateway-identity.h"
#include "lorawan/storage/serialization/gateway-serialization.h"
#include "lorawan/storage/serialization/service-serialization.h"

/**
 * request gateway identifier(with address) by IP address
 *      { "tag": "A[ddress]"}
 * return
 *  { "gwid": "", "addr":"" }
 * or
 *  {"code: <number> } if fail
 * request IP address (with identifier) by gateway identifier
 *      { "tag": "I[dentifier]", "gwid": "<gateway id (hex number)>"}
 * return
 *  { "gwid": "", "addr":"" }
 * or
 *  {"code: <number> } if fail
 * request list
 *      { "tag": "L[ist]", "offset": <number>, "size": <number>}
 *  Offset 0..N, size 0..255
 * return
 *  [{ "gwid": "", "addr":"" }, ...]
 * request count
 *      { "tag": "C[ount]"}
 * return <number>
 * put gateway
 *      { "tag": "P[ut]", "addr":"", "gwid":""}
 * return
 *  {"code: <number> } 0- success, otherwise error code
 * remove gateway
 *      { "tag": "R[emove]", "addr": "<address>", "gwid":""}
 * return
 *  {"code: <number> } 0- success, otherwise error code
 * force save
 *      { "tag": "S[ave]"}
 * return
 *  {"code: 0 } not implemented
 * close resources
 *      { "tag": "E[nd]"}
 * return
 *  {"code: 0 } not implemented
 */
class GatewayTextJSONSerialization : public GatewaySerialization {
public:
    explicit GatewayTextJSONSerialization(
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

#endif
