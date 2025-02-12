#ifndef IDENTITY_TEXT_JSON_SERIALIZATION_H
#define IDENTITY_TEXT_JSON_SERIALIZATION_H

#include <cinttypes>
#include "lorawan/storage/service/identity-service.h"
#include "lorawan/storage/serialization/identity-serialization.h"
#include "lorawan/storage/serialization/service-serialization.h"

/**
 * request identifier(with address) by network address
 *      { "tag": "a[ddress]", "eui": "<EUI>"}
 *      { "tag": "a", "addr": "<address>"}
 * return
 *  { "addr":"", "activation":"","class":"","deveui":"","nwkSKey":"","appSKey":"",
 *      "version":"","appeui":"","appKey":"","nwkKey":"",
 *      "devNonce":"","joinNonce":"","name":""}
 * request address (with identifier) by identifier
 *      { "tag": "i[dentifier]", "addr": "<address>"}
 * return
 *  { "addr":"", "activation":"","class":"","deveui":"","nwkSKey":"","appSKey":"",
 *      "version":"","appeui":"","appKey":"","nwkKey":"",
 *      "devNonce":"","joinNonce":"","name":""}
 * or {"code: <number> } if fail
 * request list
 *      { "tag": "l[ist]", "offset": <number>, "size": <number>}
 *  Offset 0..N, size 0..255
 * return
 *  [{ "addr":"", "activation":"","class":"","deveui":"","nwkSKey":"","appSKey":"",...}, ...]
 * request count
 *      { "tag": "c[ount]"}
 * return <number>
 * request next
 *      { "tag": "n[ext]"}
 * return
 *  { "addr":"", "activation":"","class":"","deveui":"","nwkSKey":"","appSKey":"",...}
 * or {"code: <number> } if fail
 * put identity
 *      { "tag": "p[ut]", "addr":"", "activation":"","class":"","deveui":"","nwkSKey":"","appSKey":"",
 *      "version":"","appeui":"","appKey":"","nwkKey":"",
 *      "devNonce":"","joinNonce":"","name":""}
 * return
 *  {"code: <number> } 0- success, otherwise error code
 * remove address
 *      { "tag": "r[emove]", "addr": "<address>"}
 * return
 *  {"code: <number> } 0- success, otherwise error code
 *
 * force save
 *      { "tag": "s[ave]"}
 * return
 *  {"code: 0 } not implemented
 * close resources
 *      { "tag": "e[nd]"}
 * return
 *  {"code: 0 } not implemented
 */
class IdentityTextJSONSerialization : public IdentitySerialization {
public:
    explicit IdentityTextJSONSerialization(
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
