#ifndef IDENTITY_TEXT_TAB_SERIALIZATION_H
#define IDENTITY_TEXT_TAB_SERIALIZATION_H

#include <cinttypes>
#include "lorawan/storage/service/identity-service.h"
#include "lorawan/storage/serialization/identity-serialization.h"
#include "lorawan/storage/serialization/service-serialization.h"

/**
 * No credentials checking implemented
 *
 * request identifier(with address) by network address
 *      a[ddress] <EUI>
 *      a[ddress] <address>
 * return
 *  { "addr":"", "activation":"","class":"","deveui":"","nwkSKey":"","appSKey":"",
 *      "version":"","appeui":"","appKey":"","nwkKey":"",
 *      "devNonce":"","joinNonce":"","name":""}
 * request address (with identifier) by identifier
 *      i[dentifier]    <address>
 * return
 *  { "addr":"", "activation":"","class":"","deveui":"","nwkSKey":"","appSKey":"",
 *      "version":"","appeui":"","appKey":"","nwkKey":"",
 *      "devNonce":"","joinNonce":"","name":""}
 * or {"code: <number> } if fail
 * request list
 *      l[ist]  <offset-number> <size-number>
 *  Offset 0..N, size 0..255
 * return
 *  [{ "addr":"", "activation":"","class":"","deveui":"","nwkSKey":"","appSKey":"",...}, ...]
 * request count
 *      c[ount]
 * return <number>
 * request next
 *      n[ext]
 * return
 *  { "addr":"", "activation":"","class":"","deveui":"","nwkSKey":"","appSKey":"",...}
 * or {"code: <number> } if fail
 * put identity
 *      p[ut] <addr> <activation> <class> <deveui> <nwkSKey> <appSKey> <version> <appeui> <appKey> <nwkKey> <devNonce> <joinNonce> <name>
 * return
 *  {"code: <number> } 0- success, otherwise error code
 * remove address
 *      r[emove] <address>
 * return
 *  {"code: <number> } 0- success, otherwise error code
 *
 * force save
 *      s[ave]
 * return
 *  {"code: 0 } not implemented
 * close resources
 *      e[nd]
 * return
 *  {"code: 0 } not implemented
 */
class IdentityTextTabSerialization : public IdentitySerialization {
public:
    explicit IdentityTextTabSerialization(
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
