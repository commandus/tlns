#include <sstream>
#include <cstring>

#include "lorawan/storage/serialization/identity-binary-serialization.h"
#include "lorawan/lorawan-conv.h"

#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"

#ifdef ESP_PLATFORM
#include "platform-defs.h"
#endif

#ifdef ENABLE_DEBUG
#include <iostream>
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-msg.h"
#endif

static void serializeNETWORKIDENTITY(
    unsigned char* retBuf,
    const NETWORKIDENTITY &response
)
{
    unsigned char *p = retBuf;
    memmove(p, &response.devaddr.u, sizeof(uint32_t));      // 4
    p += sizeof(uint32_t);
    *(uint8_t *) p = (uint8_t) response.devid.activation;	// 1 activation type: ABP or OTAA
    p++;
    *(uint8_t *) p = (uint8_t) response.devid.deviceclass;	// 1 DEVICECLASS: A, B. C
    p++;
    ((DEVEUI *) p)->u = response.devid.devEUI.u;	        // 8 device identifier (ABP device may not store EUI) (14)
    p += 8;
    memmove(p, &response.devid.nwkSKey, 16);	            // 16 shared session key
    p += 16;
    memmove(p, &response.devid.appSKey, 16);	            // 16 private key
    p += 16;
    *(uint8_t *) p = response.devid.version.c;	            // 1 LORAWAN_VERSION 1
    p++;
    // OTAA
    ((DEVEUI *) p)->u = response.devid.appEUI.u;	        // 8 OTAA application identifier
    p += 8;
    memmove(p, &response.devid.appKey, 16);	                // 16 OTAA application private key
    p += 16;
    memmove(p, &response.devid.nwkKey, 16);	                // 16 OTAA network key
    p += 16;
    ((DEVNONCE *) p)->u = response.devid.devNonce.u;	    // 2 last device nonce (87)
    p += 2;
    memmove(p, &response.devid.joinNonce.c, 3);	// 3 Join nonce
    p += 3;

    // added for searching
    memmove(p, &response.devid.name, 8);	                // 8 total 141
}

static void deserializeNETWORKIDENTITY(
    NETWORKIDENTITY &retVal,
    const unsigned char* buf
)
{
    memmove(&retVal.devaddr.u, buf, sizeof(uint32_t)); // 4
    unsigned char *p = (unsigned char *) buf + sizeof(uint32_t);
    retVal.devid.activation = (ACTIVATION) *(uint8_t *) p;	    // 1 activation type: ABP or OTAA
    p++;
    retVal.devid.deviceclass = (DEVICECLASS) *(uint8_t *) p;	// 1 Device class: A, B, C
    p++;
    retVal.devid.devEUI.u = ((DEVEUI *) p)->u;	    // 8 device identifier (ABP device may not store EUI)
    p += 8;
    memmove(&retVal.devid.nwkSKey.c, p, 16);	        // 16 shared session key
    p += 16;
    memmove(&retVal.devid.appSKey.c, p, 16);	        // 16 private key
    p += 16;
    retVal.devid.version = (LORAWAN_VERSION) *(uint8_t *) p;	// 1
    p++;
    // OTAA
    retVal.devid.appEUI.u = ((DEVEUI *) p)->u;	    // 8 OTAA application identifier
    p += 8;
    memmove(&retVal.devid.appKey.c, p, 16);	        // 16 OTAA application private key
    p += 16;
    memmove(&retVal.devid.nwkKey.c, p, 16);	        // 16 OTAA network key
    p += 16;
    retVal.devid.devNonce.u = ((DEVNONCE *) p)->u;	// 2 last device nonce
    p += 2;
    memmove(&retVal.devid.joinNonce.c, &(((JOINNONCE *) p)->c), 3);	// 3 Join nonce
    p += 3;
    // added for searching
    memmove(&retVal.devid.name, p, 8);	            // 8
}

IdentityEUIRequest::IdentityEUIRequest()
    : ServiceMessage(QUERY_IDENTITY_ADDR, 0, 0), eui(0)
{
}

IdentityEUIRequest::IdentityEUIRequest(
    char aTag,
    const DEVEUI &aEUI,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(aTag, code, accessCode), eui(aEUI)
{

}

IdentityEUIRequest::IdentityEUIRequest(
    const unsigned char *buf,
    size_t sz
)
    : ServiceMessage(buf, sz)   //  13
{
    if (sz >= SIZE_DEVICE_EUI_REQUEST) {
        memmove(&eui.u, &buf[13], sizeof(eui.u));     // 8
    }   // 21
}

void IdentityEUIRequest::ntoh() {
    ServiceMessage::ntoh();
    eui.u = NTOH8(eui.u);
}

size_t IdentityEUIRequest::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);      // 13
    memmove(&retBuf[13], &eui.u, sizeof(eui.u));  // 8
    return SIZE_DEVICE_EUI_REQUEST;         // 21
}

std::string IdentityEUIRequest::toJsonString() const
{
    std::stringstream ss;
    ss << R"("eui":")" << DEVEUI2string(eui);
    return ss.str();
}

IdentityAddrRequest::IdentityAddrRequest()
    : ServiceMessage(QUERY_IDENTITY_ADDR, 0, 0)
{
    memset(&addr.u, 0, sizeof(addr.u));
}

IdentityAddrRequest::IdentityAddrRequest(
    const unsigned char *buf,
    size_t sz
)
    : ServiceMessage(buf, sz)
{
    if (sz >= SIZE_DEVICE_ADDR_REQUEST) {
        memmove(&addr.u, buf + SIZE_SERVICE_MESSAGE, sizeof(addr.u)); // 4
    }
}

IdentityAddrRequest::IdentityAddrRequest(
    char aTag,
    const DEVADDR &aAddr,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(aTag, code, accessCode)
{
    addr.u = aAddr.u;
}

void IdentityAddrRequest::ntoh()
{
    ServiceMessage::ntoh();
    addr.u = NTOH4(addr.u);
}

size_t IdentityAddrRequest::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);      // 13
    memmove(retBuf + SIZE_SERVICE_MESSAGE, &addr.u, sizeof(addr.u)); // 4
    return SIZE_DEVICE_ADDR_REQUEST;        // 17
}

std::string IdentityAddrRequest::toJsonString() const
{
    std::stringstream ss;
    ss << R"({"addr": ")" << DEVADDR2string(addr) << "\"}";
    return ss.str();
}

IdentityAssignRequest::IdentityAssignRequest()
    : ServiceMessage(QUERY_IDENTITY_ASSIGN, 0, 0), identity()
{
}

IdentityAssignRequest::IdentityAssignRequest(
    const unsigned char *buf,
    size_t sz
)
    : ServiceMessage(buf, sz)   // 13
{
    if (sz >= SIZE_ASSIGN_REQUEST) {
        memmove(&identity.devaddr.u, buf + SIZE_SERVICE_MESSAGE, sizeof(identity.devaddr.u));   // 4
        deserializeNETWORKIDENTITY(identity, buf + SIZE_SERVICE_MESSAGE);
    }   // 108
}

IdentityAssignRequest::IdentityAssignRequest(
    char aTag,
    const NETWORKIDENTITY &aIdentity,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(aTag, code, accessCode), identity(aIdentity)
{
}

void IdentityAssignRequest::ntoh()
{
    ServiceMessage::ntoh();
    identity.devaddr.u = NTOH4(identity.devaddr.u);
    identity.devid.devEUI.u = NTOH8(identity.devid.devEUI.u);
}

size_t IdentityAssignRequest::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);      // 13
    serializeNETWORKIDENTITY(retBuf + SIZE_SERVICE_MESSAGE, identity);  // 141
    return SIZE_ASSIGN_REQUEST;                           // 13 + 141 = 154
}

std::string IdentityAssignRequest::toJsonString() const
{
    std::stringstream ss;
    ss << R"({"identity": ")" << identity.toJsonString() << "\"}";
    return ss.str();
}

IdentityOperationRequest::IdentityOperationRequest()
    : ServiceMessage(QUERY_IDENTITY_LIST, 0, 0),
      offset(0), size(0)
{
}

IdentityOperationRequest::IdentityOperationRequest(
    char tag,
    uint32_t aOffset,
    uint8_t aSize,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(tag, code, accessCode), offset((uint32_t) aOffset), size((uint8_t) aSize)
{
}

IdentityOperationRequest::IdentityOperationRequest(
    const unsigned char *buf,
    size_t sz
)
    : ServiceMessage(buf, sz)   // 13
{
    if (sz >= SIZE_OPERATION_REQUEST) {
        memmove(&offset, &buf[13], sizeof(offset));     // 4
        memmove(&size, &buf[17], sizeof(size));         // 1
    }   // 18
}

void IdentityOperationRequest::ntoh() {
    ServiceMessage::ntoh();
    offset = NTOH4(offset);
}

size_t IdentityOperationRequest::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);                  // 13
    if (retBuf) {
        memmove(&retBuf[13], &offset, sizeof(offset));  // 4
        memmove(&retBuf[17], &size, sizeof(size));      // 1
    }
    return SIZE_OPERATION_REQUEST;                      // 18
}

std::string IdentityOperationRequest::toJsonString() const {
    std::stringstream ss;
    ss << R"({"offset": )" << offset
        << ", \"size\": " << (int) size
        << "}";
    return ss.str();
}

IdentityGetResponse::IdentityGetResponse(
    const IdentityAddrRequest& req
)
    : ServiceMessage(req), response(req.addr)
{
}

IdentityGetResponse::IdentityGetResponse(
    const IdentityEUIRequest &request
)
    : ServiceMessage(request), response(request.eui)
{
    tag = request.tag;
    code = request.code;
    accessCode = request.accessCode;
}

static void ntohNETWORKIDENTITY(
    NETWORKIDENTITY &value
)
{
    value.devaddr.u = NTOH4(value.devaddr.u);
    value.devid.devEUI.u = NTOH8(value.devid.devEUI.u);
    // OTAA
    value.devid.appEUI.u = NTOH8(value.devid.appEUI.u);
    value.devid.devNonce.u = NTOH2(value.devid.devNonce.u);
}

IdentityGetResponse::IdentityGetResponse(
    const unsigned char* buf,
    size_t sz
)
    : ServiceMessage(buf, sz)   // 13
{
    if (sz >= SIZE_GET_RESPONSE) {
        deserializeNETWORKIDENTITY(response, buf + 13);
    }
}

std::string IdentityGetResponse::toJsonString() const {
    std::stringstream ss;
    ss << response.toJsonString();
    return ss.str();
}

void IdentityGetResponse::ntoh()
{
    ServiceMessage::ntoh();
    ntohNETWORKIDENTITY(response);
}

size_t IdentityGetResponse::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);                   // 13
    serializeNETWORKIDENTITY(retBuf + SIZE_SERVICE_MESSAGE, this->response);
    return SIZE_GET_RESPONSE;                           // 13 + 95 = 108
}

IdentityNextResponse::IdentityNextResponse(
    const unsigned char* buf,
    size_t sz
)
    : ServiceMessage(buf, sz)   // 13
{
    if (sz >= SIZE_GET_RESPONSE) {
        deserializeNETWORKIDENTITY(response, buf + 13);
    }
}

std::string IdentityNextResponse::toJsonString() const {
    std::stringstream ss;
    ss << response.toJsonString();
    return ss.str();
}

void IdentityNextResponse::ntoh()
{
    ServiceMessage::ntoh();
    ntohNETWORKIDENTITY(response);
}

size_t IdentityNextResponse::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);                   // 13
    serializeNETWORKIDENTITY(retBuf + SIZE_SERVICE_MESSAGE, this->response);
    return SIZE_GET_RESPONSE;                           // 13 + 95 = 108
}

IdentityOperationResponse::IdentityOperationResponse()
    : IdentityOperationRequest(), response(0)
{
}

IdentityOperationResponse::IdentityOperationResponse(
    const IdentityOperationResponse& resp
)
    : IdentityOperationRequest(resp), response(resp.response)
{
}

IdentityOperationResponse::IdentityOperationResponse(
    const unsigned char *buf,
    size_t sz
)
    : IdentityOperationRequest(buf, sz) // 18
{
    if (sz >= SIZE_OPERATION_RESPONSE) {
        memmove(&response, buf + SIZE_OPERATION_REQUEST, sizeof(response)); // 4
    }   // 22
}

IdentityOperationResponse::IdentityOperationResponse(
    const IdentityAddrRequest &request
)
    : IdentityOperationRequest(request.tag, 0, 0, request.code, request.accessCode), response(0)
{
}


IdentityOperationResponse::IdentityOperationResponse(
    const IdentityAssignRequest &request
)
    : IdentityOperationRequest(request.tag, 0, 0, request.code, request.accessCode), response(0)
{
}

IdentityOperationResponse::IdentityOperationResponse(
    const IdentityOperationRequest &request
)
    : IdentityOperationRequest(request), response(0)
{

}

void IdentityOperationResponse::ntoh() {
    IdentityOperationRequest::ntoh();
    response = NTOH4(response);
}

size_t IdentityOperationResponse::serialize(
    unsigned char *retBuf
) const
{
    IdentityOperationRequest::serialize(retBuf);                                // 18
    if (retBuf)
        memmove(retBuf + SIZE_OPERATION_REQUEST, &response, sizeof(response));  // 4
    return SIZE_OPERATION_RESPONSE;                                             // 22
}

std::string IdentityOperationResponse::toJsonString() const {
    std::stringstream ss;
    ss << R"({"request": )" << IdentityOperationRequest::toJsonString()
       << ", \"response\": " << response << "}";
    return ss.str();
}

IdentityListResponse::IdentityListResponse()
    : IdentityOperationResponse()
{
}

IdentityListResponse::IdentityListResponse(
    const IdentityListResponse& resp
)
    : IdentityOperationResponse(resp)
{
}

IdentityListResponse::IdentityListResponse(
    const unsigned char *buf,
    size_t sz
)
    : IdentityOperationResponse(buf, sz)       // 22
{
    deserializeIdentity(buf, sz);
    size_t ofs = SIZE_OPERATION_RESPONSE;
    response = 0;
    while (true) {
        if (ofs + SIZE_NETWORK_IDENTITY > sz)
            break;
        NETWORKIDENTITY ni;
        deserializeNETWORKIDENTITY(ni, buf + ofs);
        ofs += SIZE_NETWORK_IDENTITY;
        identities.push_back(ni);
        response++;
    }
}

IdentityListResponse::IdentityListResponse(
    const IdentityOperationRequest &request
)
    : IdentityOperationResponse(request)
{
}

void IdentityListResponse::ntoh()
{
    IdentityOperationResponse::ntoh();
    for (auto it(identities.begin()); it != identities.end(); it++) {
        ntohNETWORKIDENTITY(*it);
    }
}

size_t IdentityListResponse::serialize(
    unsigned char *retBuf
) const
{
    size_t ofs = IdentityOperationResponse::serialize(retBuf);   // 22
    if (retBuf) {
        for (auto it(identities.begin()); it != identities.end(); it++) {
            serializeNETWORKIDENTITY(retBuf + ofs, *it);
            ofs += SIZE_NETWORK_IDENTITY;
        }
    } else {
        ofs += SIZE_NETWORK_IDENTITY * identities.size();
    }
    return ofs;
}

std::string IdentityListResponse::toJsonString() const {
    std::stringstream ss;
    ss << R"({"result": )" << IdentityOperationResponse::toJsonString()
       << ", \"identities\": [";
    bool isFirst = true;
    for (auto i = 0; i < response; i++) {
        if (isFirst)
            isFirst = false;
        else
            ss << ", ";
        ss << identities[i].toJsonString();
    }
    ss <<  "]}";
    return ss.str();
}

size_t IdentityListResponse::shortenList2Fit(
    size_t serializedSize
) {
    size_t r = this->serialize(nullptr);
    while((!identities.empty()) && (r > serializedSize)) {
        identities.erase(identities.end() - 1);
        r = this->serialize(nullptr);
    }
    return r;
}

/**
 * Get size for serialized list
 * @param sz count ofg items
 * @return size in bytes
 */
static size_t getMaxIdentityListResponseSize(
    size_t sz
)
{
    return SIZE_OPERATION_RESPONSE + sz * SIZE_NETWORK_IDENTITY;
}

size_t IdentitySerialization::query(
    unsigned char *retBuf,
    size_t retSize,
    const unsigned char *request,
    size_t sz
)
{
    if (!svc)
        return 0;
    if (sz < SIZE_SERVICE_MESSAGE)
        return 0;
    ServiceMessage *pMsg = deserializeIdentity((const unsigned char *) request, sz);
    if (!pMsg)
        return 0;   // unknown request
    if ((pMsg->code != code) || (pMsg->accessCode != accessCode)) {
#ifdef ENABLE_DEBUG
        std::cerr << ERR_ACCESS_DENIED
            << ": " << pMsg->code
            << "," << pMsg->accessCode
            << std::endl;
#endif
        auto r = new IdentityOperationResponse;
        r->code = ERR_CODE_ACCESS_DENIED;
        r->ntoh();
        size_t sz = r->serialize(retBuf);
        delete pMsg;
        return sz;
    }

    ServiceMessage *r = nullptr;
    switch (request[0]) {
        case QUERY_IDENTITY_ADDR:   // request device identifier(with address) by network address. Return 0 if success
            {
                auto gr = (IdentityEUIRequest *) pMsg;
                r = new IdentityGetResponse(*gr);
                ((IdentityGetResponse*) r)->response.devid.devEUI.u = gr->eui.u;
                int errCode;
                if (((IdentityGetResponse*) r)->response.devaddr.empty())
                    errCode = svc->getNetworkIdentity(((IdentityGetResponse*) r)->response,
                        ((IdentityGetResponse*) r)->response.devid.devEUI);
                else
                    errCode = svc->get(((IdentityGetResponse*) r)->response.devid, ((IdentityGetResponse*) r)->response.devaddr);

                if (errCode) {
                    // indicate nothing there
                    ((IdentityGetResponse*) r)->response.devid.devEUI.u = 0;
                }
                break;
            }
        case QUERY_IDENTITY_EUI:   // request device address (with identifier) by identifier. Return 0 if success
            {
                auto gr = (IdentityAddrRequest *) pMsg;
                r = new IdentityGetResponse(*gr);
                ((IdentityGetResponse*) r)->response.devaddr.u = gr->addr.u;
                svc->get(((IdentityGetResponse*) r)->response.devid, ((IdentityGetResponse*) r)->response.devaddr);
                break;
            }
        case QUERY_IDENTITY_ASSIGN:   // assign (put) device address to the gateway by identifier
            {
                auto gr = (IdentityAssignRequest *) pMsg;
                r = new IdentityOperationResponse(*gr);
                ((IdentityOperationResponse*) r)->response = svc->put(gr->identity.devaddr, gr->identity.devid);
                if (((IdentityOperationResponse*) r)->response == 0)
                    ((IdentityOperationResponse *) r)->size = 1;    // count of placed entries
                break;
            }
        case QUERY_IDENTITY_RM:   // Remove entry
            {
                auto gr = (IdentityAddrRequest *) pMsg;
                r = new IdentityOperationResponse(*gr);
                ((IdentityOperationResponse*) r)->response = svc->rm(gr->addr);
                if (((IdentityOperationResponse*) r)->response == 0)
                    ((IdentityOperationResponse *) r)->size = 1;    // count of deleted entries
                break;
            }
        case QUERY_IDENTITY_LIST:   // List entries
        {
            auto gr = (IdentityOperationRequest *) pMsg;
            r = new IdentityListResponse(*gr);
            ((IdentityOperationResponse*) r)->response = svc->list(((IdentityListResponse *) r)->identities, gr->offset, gr->size);
            size_t idSize = ((IdentityListResponse *) r)->identities.size();
            size_t serSize = SIZE_OPERATION_RESPONSE + (idSize * SIZE_NETWORK_IDENTITY);
            if (serSize > retSize) {
                serSize = ((IdentityListResponse *) r)->shortenList2Fit(retSize);
                if (serSize > retSize) {
                    delete r;
                    r = nullptr;
                }
                break;
            }
            break;        }
        case QUERY_IDENTITY_COUNT:   // count
        {
            auto gr = (IdentityOperationRequest *) pMsg;
            r = new IdentityOperationResponse(*gr);
            r->tag = gr->tag;
            r->code = CODE_OK;
            r->accessCode = gr->accessCode;
            ((IdentityOperationResponse *) r)->response = (uint32_t) svc->size();
            break;
        }
        case QUERY_IDENTITY_NEXT:   // next
        {
            auto gr = (IdentityOperationRequest *) pMsg;
            r = new IdentityNextResponse;
            r->tag = gr->tag;
            r->code = CODE_OK;
            r->accessCode = gr->accessCode;
            NETWORKIDENTITY ni;
            svc->next(ni);
            ((IdentityNextResponse *) r)->response = ni;
            break;
        }
        case QUERY_IDENTITY_FORCE_SAVE:   // force save
            break;
        case QUERY_IDENTITY_CLOSE_RESOURCES:   // close resources
            break;
        default:
            break;
    }
    delete pMsg;
    size_t rsize = 0;
    if (r) {
        r->ntoh();
        rsize = r->serialize(retBuf);
        delete r;
    }
    return rsize;
}

/**
 * Check does it serialized query in the buffer
 * @param buffer buffer to check
 * @param size buffer size
 * @return query tag
 */
enum IdentityQueryTag validateIdentityQuery(
    const unsigned char *buffer,
    size_t size
)
{
    if (size == 0)
        return QUERY_IDENTITY_NONE;
    switch (buffer[0]) {
        case QUERY_IDENTITY_ADDR:   // request gateway identifier(with address) by network address.
            if (size < SIZE_DEVICE_EUI_REQUEST)
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_ADDR;
        case QUERY_IDENTITY_EUI:   // request gateway address (with identifier) by identifier.
            if (size < SIZE_DEVICE_ADDR_REQUEST)
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_EUI;
        case QUERY_IDENTITY_ASSIGN:   // assign (put) gateway address to the gateway by identifier
            if (size < SIZE_DEVICE_ADDR_REQUEST)
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_ASSIGN;
        case QUERY_IDENTITY_RM:   // Remove entry
            if (size < SIZE_DEVICE_ADDR_REQUEST)
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_RM;
        case QUERY_IDENTITY_LIST:   // List entries
            if (size < SIZE_OPERATION_REQUEST)
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_LIST;
        case QUERY_IDENTITY_COUNT:   // count
            if (size < SIZE_OPERATION_REQUEST)
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_COUNT;
        case QUERY_IDENTITY_NEXT:   // next
            if (size < SIZE_OPERATION_REQUEST)
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_NEXT;
        case QUERY_IDENTITY_FORCE_SAVE:   // force save
            if (size < SIZE_OPERATION_REQUEST)
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_FORCE_SAVE;
        case QUERY_IDENTITY_CLOSE_RESOURCES:   // close resources
            if (size < SIZE_OPERATION_REQUEST)
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_CLOSE_RESOURCES;
    default:
            break;
    }
    return QUERY_IDENTITY_NONE;
}

/**
 * Check does it serialized response in the buffer
 * @param buffer buffer to check
 * @param size buffer size
 * @return query tag
 */
enum IdentityQueryTag validateIdentityResponse(
    const unsigned char *buffer,
    size_t size
)
{
    if (size == 0)
        return QUERY_IDENTITY_NONE;
    switch (buffer[0]) {
        case QUERY_IDENTITY_ADDR:   // request gateway identifier(with address) by network address.
            if (size < SIZE_GET_RESPONSE)
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_ADDR;
        case QUERY_IDENTITY_EUI:   // request gateway address (with identifier) by identifier.
            if (size < SIZE_GET_RESPONSE)
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_EUI;
        case QUERY_IDENTITY_ASSIGN:   // assign (put) gateway address to the gateway by identifier
            if (size < SIZE_OPERATION_RESPONSE)
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_ASSIGN;
        case QUERY_IDENTITY_RM:   // Remove entry
            if (size < SIZE_OPERATION_RESPONSE)
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_RM;
        case QUERY_IDENTITY_LIST:   // List entries
            if (size < SIZE_GET_RESPONSE)   // at least
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_LIST;
        case QUERY_IDENTITY_COUNT:   // count
            if (size < SIZE_OPERATION_RESPONSE)
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_COUNT;
        case QUERY_IDENTITY_NEXT:   // next
            if (size < SIZE_OPERATION_RESPONSE)
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_NEXT;
        case QUERY_IDENTITY_FORCE_SAVE:   // force save
            if (size < SIZE_OPERATION_RESPONSE)
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_FORCE_SAVE;
        case QUERY_IDENTITY_CLOSE_RESOURCES:   // close resources
            if (size < SIZE_OPERATION_RESPONSE)
                return QUERY_IDENTITY_NONE;
            return QUERY_IDENTITY_CLOSE_RESOURCES;
        default:
            break;
    }
    return QUERY_IDENTITY_NONE;
}

/**
 * Return required size for response
 * @param buffer serialized request
 * @param size buffer size
 * @return size in bytes
 */
size_t responseSizeForIdentityRequest(
    const unsigned char *buffer,
    size_t size
)
{
    enum IdentityQueryTag tag = validateIdentityQuery(buffer, size);
    switch (tag) {
        case QUERY_IDENTITY_ADDR:       // request gateway identifier(with address) by network address.
        case QUERY_IDENTITY_EUI:        // request gateway address (with identifier) by identifier.
            return SIZE_GET_RESPONSE;   //
        case QUERY_IDENTITY_LIST:       // List entries
            {
                IdentityOperationRequest lr(buffer, size);
                return getMaxIdentityListResponseSize(lr.size);
            }
        default:
            break;
    }
    return SIZE_OPERATION_RESPONSE; // IdentityOperationResponse
}

/**
 * Return request object or NULL if packet is invalid
 * @param buf buffer
 * @param sz buffer size
 * @return return NULL if packet is invalid
 */
ServiceMessage* deserializeIdentity(
    const unsigned char *buf,
    size_t sz
)
{
    if (sz < SIZE_SERVICE_MESSAGE)
        return nullptr;
    ServiceMessage *r;
    switch (buf[0]) {
        case QUERY_IDENTITY_ADDR:   // request gateway identifier(with address) by network address. Return 0 if success
            if (sz < SIZE_DEVICE_EUI_REQUEST)
                return nullptr;
            r = new IdentityEUIRequest(buf, sz);
            break;
        case QUERY_IDENTITY_EUI:   // request gateway address (with identifier) by identifier. Return 0 if success
            if (sz < SIZE_DEVICE_ADDR_REQUEST)
                return nullptr;
            r = new IdentityAddrRequest(buf, sz);
            break;
        case QUERY_IDENTITY_ASSIGN:   // assign (put) gateway address to the gateway by identifier
            if (sz < SIZE_ASSIGN_REQUEST)
                return nullptr;
            r = new IdentityAssignRequest(buf, sz);
            break;
        case QUERY_IDENTITY_RM:   // Remove entry
            if (sz < SIZE_DEVICE_ADDR_REQUEST)   // it can contain id only(no address)
                return nullptr;
            r = new IdentityAddrRequest(buf, sz);
            break;
        case QUERY_IDENTITY_LIST:   // List entries
            if (sz < SIZE_OPERATION_REQUEST)
                return nullptr;
            r = new IdentityOperationRequest(buf, sz);
            break;
        case QUERY_IDENTITY_COUNT:   // count
            if (sz < SIZE_OPERATION_REQUEST)
                return nullptr;
            r = new IdentityOperationRequest(buf, sz);
            break;
        case QUERY_IDENTITY_NEXT:   // next
            if (sz < SIZE_OPERATION_REQUEST)
                return nullptr;
            r = new IdentityOperationRequest(buf, sz);
            break;
        case QUERY_IDENTITY_FORCE_SAVE:   // force save
            if (sz < SIZE_OPERATION_REQUEST)
                return nullptr;
            r = new IdentityOperationRequest(buf, sz);
            break;
        case QUERY_IDENTITY_CLOSE_RESOURCES:   // close resources
            if (sz < SIZE_OPERATION_REQUEST)
                return nullptr;
            r = new IdentityOperationRequest(buf, sz);
            break;
        default:
            r = nullptr;
    }
    if (r)
        r->ntoh();
    return r;
}

const char* identityTag2string(
    enum IdentityQueryTag value
)
{
    switch (value) {
        case QUERY_IDENTITY_ADDR:
            return "address";
        case QUERY_IDENTITY_EUI:
            return "identifier";
        case QUERY_IDENTITY_LIST:
            return "list";
        case QUERY_IDENTITY_COUNT:
            return "count";
        case QUERY_IDENTITY_NEXT:
            return "next";
        case QUERY_IDENTITY_ASSIGN:
            return "assign";
        case QUERY_IDENTITY_RM:
            return "remove";
        case QUERY_IDENTITY_FORCE_SAVE:
            return "save";
        case QUERY_IDENTITY_CLOSE_RESOURCES:
            return "close";
        default:
            return "";
    }
}

static std::string IDCS("ailcprse");

const std::string &identityCommandSet() {
    return IDCS;
}

/**
 * Check does it identity tag in the buffer
 * @param buffer buffer to check
 * @param size buffer size
 * @return true
 */
bool isIdentityTag(
    const unsigned char *buffer,
    size_t size
)
{
    if (size == 0)
        return false;
    switch (buffer[0]) {
        case QUERY_IDENTITY_ADDR:
        case QUERY_IDENTITY_EUI:
        case QUERY_IDENTITY_LIST:
        case QUERY_IDENTITY_COUNT:
        case QUERY_IDENTITY_ASSIGN:
        case QUERY_IDENTITY_RM:
        case QUERY_IDENTITY_FORCE_SAVE:
        case QUERY_IDENTITY_CLOSE_RESOURCES:
            return true;
        default:
            return false;
    }
}


IdentityBinarySerialization::IdentityBinarySerialization(
    IdentityService* aSvc,
    int32_t aCode,
    uint64_t aAccessCode
)
    : IdentitySerialization(SKT_BINARY, aSvc, aCode, aAccessCode)
{

}

size_t IdentityBinarySerialization::query(
    unsigned char* retBuf,
    size_t retSize,
    const unsigned char* request,
    size_t sz
)
{
    if (!svc)
        return 0;
    if (sz < SIZE_SERVICE_MESSAGE)
        return 0;
    ServiceMessage* pMsg = deserializeIdentity((const unsigned char*)request, sz);
    if (!pMsg)
        return 0;   // unknown request
    if ((pMsg->code != code) || (pMsg->accessCode != accessCode)) {
#ifdef ENABLE_DEBUG
        std::cerr << ERR_ACCESS_DENIED
            << ": " << pMsg->code
            << "," << pMsg->accessCode
            << std::endl;
#endif
        auto r = new IdentityOperationResponse;
        r->code = ERR_CODE_ACCESS_DENIED;
        r->ntoh();
        size_t sz = r->serialize(retBuf);
        delete pMsg;
        return sz;
    }

    ServiceMessage* r = nullptr;
    switch (request[0]) {
    case QUERY_IDENTITY_ADDR:   // request gateway identifier(with address) by network address. Return 0 if success
    {
        auto gr = (IdentityEUIRequest*)pMsg;
        r = new IdentityGetResponse(*gr);
        ((IdentityGetResponse*)r)->response.devid.devEUI.u = gr->eui.u;
        int errCode;
        if (((IdentityGetResponse*)r)->response.devaddr.empty())
            errCode = svc->getNetworkIdentity(((IdentityGetResponse*)r)->response,
                ((IdentityGetResponse*)r)->response.devid.devEUI);
        else
            errCode = svc->get(((IdentityGetResponse*)r)->response.devid, ((IdentityGetResponse*)r)->response.devaddr);

        if (errCode) {
            // indicate nothing there
            ((IdentityGetResponse*)r)->response.devid.devEUI.u = 0;
        }
        break;
    }
    case QUERY_IDENTITY_EUI:   // request gateway address (with identifier) by identifier. Return 0 if success
    {
        auto gr = (IdentityAddrRequest*)pMsg;
        r = new IdentityGetResponse(*gr);
        ((IdentityGetResponse*)r)->response.devaddr.u = gr->addr.u;
        svc->get(((IdentityGetResponse*)r)->response.devid, ((IdentityGetResponse*)r)->response.devaddr);
        break;
    }
    case QUERY_IDENTITY_ASSIGN:   // assign (put) gateway address to the gateway by identifier
    {
        auto gr = (IdentityAssignRequest*)pMsg;
        r = new IdentityOperationResponse(*gr);
        ((IdentityOperationResponse*)r)->response = svc->put(gr->identity.devaddr, gr->identity.devid);
        if (((IdentityOperationResponse*)r)->response == 0)
            ((IdentityOperationResponse*)r)->size = 1;    // count of placed entries
        break;
    }
    case QUERY_IDENTITY_RM:   // Remove entry
    {
        auto gr = (IdentityAddrRequest*)pMsg;
        r = new IdentityOperationResponse(*gr);
        ((IdentityOperationResponse*)r)->response = svc->rm(gr->addr);
        if (((IdentityOperationResponse*)r)->response == 0)
            ((IdentityOperationResponse*)r)->size = 1;    // count of deleted entries
        break;
    }
    case QUERY_IDENTITY_LIST:   // List entries
    {
        auto gr = (IdentityOperationRequest*)pMsg;
        r = new IdentityListResponse(*gr);
        ((IdentityOperationResponse*)r)->response = svc->list(((IdentityListResponse*)r)->identities, gr->offset, gr->size);
        size_t idSize = ((IdentityListResponse*)r)->identities.size();
        size_t serSize = SIZE_OPERATION_RESPONSE + (idSize * SIZE_NETWORK_IDENTITY);
        if (serSize > retSize) {
            serSize = ((IdentityListResponse*)r)->shortenList2Fit(retSize);
            if (serSize > retSize) {
                delete r;
                r = nullptr;
            }
            break;
        }
        break;
    }
    case QUERY_IDENTITY_COUNT:   // count
    {
        auto gr = (IdentityOperationRequest*)pMsg;
        r = new IdentityOperationResponse(*gr);
        r->tag = gr->tag;
        r->code = CODE_OK;
        r->accessCode = gr->accessCode;
        ((IdentityOperationResponse*)r)->response = (uint8_t)svc->size();
        break;
    }
    case QUERY_IDENTITY_NEXT:   // next
    {
        auto gr = (IdentityOperationRequest*)pMsg;
        r = new IdentityNextResponse;
        r->tag = gr->tag;
        r->code = CODE_OK;
        r->accessCode = gr->accessCode;
        NETWORKIDENTITY ni;
        svc->next(ni);
        ((IdentityNextResponse*)r)->response = ni;
        break;
    }
    case QUERY_IDENTITY_FORCE_SAVE:   // force save
        break;
    case QUERY_IDENTITY_CLOSE_RESOURCES:   // close resources
        break;
    default:
        break;
    }
    delete pMsg;
    size_t rsize = 0;
    if (r) {
        r->ntoh();
        rsize = r->serialize(retBuf);
        delete r;
    }
    return rsize;
}

IdentityQueryTag isIdentityTag(const char *tag) {
    if (!tag)
        return QUERY_IDENTITY_NONE;
    const std::string &cs = identityCommandSet();
    auto len = strlen(tag);
    if (len == 1) {
        if (cs.find(*tag) != std::string::npos) {
            return (IdentityQueryTag) *tag;
        }
    } else {
        for (auto it : cs) {
            if (strcmp(identityTag2string((IdentityQueryTag) it), tag) == 0)
                return (IdentityQueryTag) it;
        }
    }
    return QUERY_IDENTITY_NONE;
}
