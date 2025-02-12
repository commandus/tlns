#include <sstream>
#include <cstring>

#include "gateway-binary-serialization.h"
#include "lorawan/helper/ip-helper.h"
#include "lorawan/helper/ip-address.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/lorawan-error.h"

#ifdef ESP_PLATFORM
#include "platform-defs.h"
#endif

#define SIZE_OPERATION_REQUEST 18
#define SIZE_OPERATION_RESPONSE 22
#define SIZE_GATEWAY_ID_REQUEST   21
#define SIZE_DEVICE_ADDR_REQUEST 20
#define SIZE_DEVICE_EUI_ADDR_REQUEST 28

#define SIZE_DEVICE_GET_ADDR_4_RESPONSE 28
#define SIZE_DEVICE_GET_ADDR_6_RESPONSE 40

#ifdef ENABLE_DEBUG
#include <iostream>
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-msg.h"
#endif

GatewayIdRequest::GatewayIdRequest()
    : ServiceMessage(QUERY_GATEWAY_ADDR, 0, 0), id(0)
{
}

GatewayIdRequest::GatewayIdRequest(
    char aTag,
    const uint64_t aId,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(aTag, code, accessCode), id(aId)
{

}

GatewayIdRequest::GatewayIdRequest(
    const unsigned char *buf,
    size_t sz
)
    : ServiceMessage(buf, sz)   //  13
{
    if (sz >= SIZE_GATEWAY_ID_REQUEST) {
        memmove(&id, &buf[13], sizeof(id));     // 8
    }   // 21
}

void GatewayIdRequest::ntoh() {
    ServiceMessage::ntoh();
    id = NTOH8(id);
}

size_t GatewayIdRequest::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);      // 13
    memmove(&retBuf[13], &id, sizeof(id));  // 8
    return SIZE_GATEWAY_ID_REQUEST;         // 21
}

std::string GatewayIdRequest::toJsonString() const
{
    std::stringstream ss;
    ss << R"("gwid":")" << std::hex << id;
    return ss.str();
}

GatewayAddrRequest::GatewayAddrRequest()
    : ServiceMessage(QUERY_GATEWAY_ID, 0, 0)
{
    memset(&addr, 0, sizeof(addr));
}

/*
GatewayAddrRequest::GatewayAddrRequest(
    const GatewayIdentity& aIdentity
)
    : ServiceMessage(QUERY_GATEWAY_ID, 0, 0)
{
    memset(&addr, 0, sizeof(addr));
}
 */

GatewayAddrRequest::GatewayAddrRequest(
    const unsigned char *buf,
    size_t sz
)
    : ServiceMessage(buf, sz)
{
    if (sz >= SIZE_SERVICE_MESSAGE) {
        deserializeSocketAddress(&addr, &buf[SIZE_SERVICE_MESSAGE], sz - SIZE_SERVICE_MESSAGE); // 0, 7, 19
    }   // IPv4: 20 IPv6: 32
}

size_t GatewayAddrRequest::serializedSize() const
{
    if (isIPv6(&addr))
        return SIZE_SERVICE_MESSAGE + 3 + 4;        // IPv4: 20 IPv6: 32
    else
        return SIZE_SERVICE_MESSAGE + 3 + 16;        // IPv4: 20 IPv6: 32
}

GatewayAddrRequest::GatewayAddrRequest(
    const struct sockaddr &aAddr,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(QUERY_GATEWAY_ID, code, accessCode)
{
    memmove(&addr, &aAddr, sizeof(addr));
}

void GatewayAddrRequest::ntoh()
{
    ServiceMessage::ntoh();
    sockaddrNtoh(&addr);
}

size_t GatewayAddrRequest::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);      // 13
    size_t r = serializeSocketAddress(&retBuf[SIZE_SERVICE_MESSAGE], &addr); // 0, 7, 19
    return SIZE_SERVICE_MESSAGE + r;        // IPv4: 20 IPv6: 32
}

std::string GatewayAddrRequest::toJsonString() const
{
    std::stringstream ss;
    ss << R"({"addr": ")" << sockaddr2string(&addr) << "\"}";
    return ss.str();
}

GatewayIdAddrRequest::GatewayIdAddrRequest()
    : ServiceMessage(QUERY_GATEWAY_ASSIGN, 0, 0), identity()
{
}

/*
GatewayIdAddrRequest::GatewayIdAddrRequest(
    const GatewayIdentity& aIdentity
)
    : ServiceMessage(QUERY_GATEWAY_ID, 0, 0), identity(aIdentity)
{
}
*/

GatewayIdAddrRequest::GatewayIdAddrRequest(
    const unsigned char *buf,
    size_t sz
)
    : ServiceMessage(buf, sz)   // 13
{
    if (sz >= SIZE_GATEWAY_ID_REQUEST) {
        memmove(&identity.gatewayId, &buf[SIZE_SERVICE_MESSAGE], sizeof(identity.gatewayId));   // 8
        deserializeSocketAddress(&identity.sockaddr, &buf[SIZE_GATEWAY_ID_REQUEST],
                                            sz - SIZE_GATEWAY_ID_REQUEST); // 0, 7, 19
    }
}

size_t GatewayIdAddrRequest::serializedSize() const
{
    return SIZE_GATEWAY_ID_REQUEST + 3 + (isIPv6(&identity.sockaddr) ? 16 : 4);     // IPv4: 28 IPv6: 40
}

GatewayIdAddrRequest::GatewayIdAddrRequest(
    char aTag,
    const GatewayIdentity &aIdentity,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(aTag, code, accessCode), identity(aIdentity)
{
}

void GatewayIdAddrRequest::ntoh()
{
    ServiceMessage::ntoh();
    identity.gatewayId = NTOH8(identity.gatewayId);
    sockaddrNtoh(&identity.sockaddr);
}


size_t GatewayIdAddrRequest::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);      // 13
    memmove(&retBuf[SIZE_SERVICE_MESSAGE], &identity.gatewayId, sizeof(identity.gatewayId)); // 8
    size_t r = serializeSocketAddress(&retBuf[SIZE_GATEWAY_ID_REQUEST], &identity.sockaddr); // 0, 7, 19
    return SIZE_GATEWAY_ID_REQUEST + r;     // IPv4: 28 IPv6: 40
}

std::string GatewayIdAddrRequest::toJsonString() const
{
    std::stringstream ss;
    ss << R"({"identity": ")" << identity.toJsonString() << "\"}";
    return ss.str();
}

GatewayOperationRequest::GatewayOperationRequest()
    : ServiceMessage(QUERY_GATEWAY_LIST, 0, 0),
      offset(0), size(0)
{
}

/*
GatewayOperationRequest::GatewayOperationRequest(
    char tag
)
    : ServiceMessage(tag, 0, 0),
      offset(0), size(0)
{
}
*/

GatewayOperationRequest::GatewayOperationRequest(
    char tag,
    size_t aOffset,
    size_t aSize,
    int32_t code,
    uint64_t accessCode
)
    : ServiceMessage(tag, code, accessCode), offset((uint32_t) aOffset), size((uint8_t) aSize)
{
}

GatewayOperationRequest::GatewayOperationRequest(
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

void GatewayOperationRequest::ntoh() {
    ServiceMessage::ntoh();
    offset = NTOH4(offset);
}

size_t GatewayOperationRequest::serialize(
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

std::string GatewayOperationRequest::toJsonString() const {
    std::stringstream ss;
    ss << R"({"offset": )" << offset
        << ", \"size\": " << (int) size
        << "}";
    return ss.str();
}

GatewayGetResponse::GatewayGetResponse(
    const GatewayAddrRequest& req
)
    : ServiceMessage(req), response()
{
}

GatewayGetResponse::GatewayGetResponse(
    const GatewayIdRequest &request
)
    : response(request.id)
{
    tag = request.tag;
    code = request.code;
    accessCode = request.accessCode;
}

/*
GatewayGetResponse::GatewayGetResponse(
    const GatewayIdAddrRequest &request
)
    : response(request.identity.gatewayId, request.identity.sockaddr)
{
    tag = request.tag;
    code = request.code;
    accessCode = request.accessCode;
}
*/

GatewayGetResponse::GatewayGetResponse(
    const unsigned char* buf,
    size_t sz
)
    : ServiceMessage(buf, sz)   // 13
{
    if (sz >= SIZE_SERVICE_MESSAGE + sizeof(uint64_t)) {
        memmove(&response.gatewayId, buf + SIZE_SERVICE_MESSAGE, sizeof(uint64_t)); // 8
        deserializeSocketAddress(&response.sockaddr, buf + SIZE_SERVICE_MESSAGE + sizeof(uint64_t), 
            sz - SIZE_SERVICE_MESSAGE - sizeof(uint64_t)); // IPv4 28 IPv6 40
    }
}

size_t GatewayGetResponse::serializedSize() const
{
    return SIZE_SERVICE_MESSAGE + sizeof(uint64_t) + 3 + (isIPv6(&response.sockaddr) ? 16 : 4); // IPv4 28 IPv6 40
}

std::string GatewayGetResponse::toJsonString() const {
    std::stringstream ss;
    ss << response.toJsonString();
    return ss.str();
}

void GatewayGetResponse::ntoh()
{
    ServiceMessage::ntoh();
    response.gatewayId = NTOH8(response.gatewayId);
    sockaddrNtoh(&response.sockaddr);
}

size_t GatewayGetResponse::serialize(
    unsigned char *retBuf
) const
{
    ServiceMessage::serialize(retBuf);                   // 13
    memmove(retBuf + SIZE_SERVICE_MESSAGE, &response.gatewayId, sizeof(uint64_t));     // 8
    return serializeSocketAddress(retBuf + SIZE_SERVICE_MESSAGE + 8, &response.sockaddr)     // 0, 7, 19
        + SIZE_SERVICE_MESSAGE + 8; // IPv4 28 IPv6 40
}

GatewayOperationResponse::GatewayOperationResponse()
    : GatewayOperationRequest(), response(0)
{
}

GatewayOperationResponse::GatewayOperationResponse(
    const GatewayOperationResponse& resp
)
    : GatewayOperationRequest(resp)
{
}

GatewayOperationResponse::GatewayOperationResponse(
    const unsigned char *buf,
    size_t sz
)
    : GatewayOperationRequest(buf, sz) // 18
{
    if (sz >= SIZE_OPERATION_RESPONSE) {
        memmove(&response, buf + SIZE_OPERATION_REQUEST, sizeof(response)); // 4
    }   // 22
}

GatewayOperationResponse::GatewayOperationResponse(
    const GatewayIdAddrRequest &request
)
    : GatewayOperationRequest(request.tag, 0, 0, request.code, request.accessCode)
{
}

GatewayOperationResponse::GatewayOperationResponse(
    const GatewayOperationRequest &request
)
    : GatewayOperationRequest(request)
{

}

void GatewayOperationResponse::ntoh() {
    GatewayOperationRequest::ntoh();
    response = NTOH4(response);
}

size_t GatewayOperationResponse::serialize(
    unsigned char *retBuf
) const
{
    GatewayOperationRequest::serialize(retBuf);                                // 18
    if (retBuf)
        memmove(retBuf + SIZE_OPERATION_REQUEST, &response,
                sizeof(response));                                      // 4
    return SIZE_OPERATION_RESPONSE;                                     // 22
}

std::string GatewayOperationResponse::toJsonString() const {
    std::stringstream ss;
    ss << R"({"request": )" << GatewayOperationRequest::toJsonString()
       << ", \"response\": " << response << "}";
    return ss.str();
}

GatewayListResponse::GatewayListResponse()
    : GatewayOperationResponse()
{
}

GatewayListResponse::GatewayListResponse(
    const GatewayListResponse& resp
)
    : GatewayOperationResponse(resp)
{
}

GatewayListResponse::GatewayListResponse(
    const unsigned char *buf,
    size_t sz
)
    : GatewayOperationResponse(buf, sz)       // 22
{
    deserializeGateway(buf, sz);
    size_t ofs = SIZE_OPERATION_RESPONSE;
    response = 0;
    while (true) {
        if (ofs >= sz)
            break;
        GatewayIdentity gi;
        memmove(&gi.gatewayId, &buf[ofs], sizeof(uint64_t));  // 8
        ofs += 8;
        ofs += deserializeSocketAddress(&gi.sockaddr, &buf[ofs], sz - ofs); // 0, 7, 19
        identities.push_back(gi);
        response++;
    }
}

size_t GatewayListResponse::serializedSize() const
{
    size_t ofs = SIZE_OPERATION_RESPONSE;
    for (auto id : identities) {
        ofs += 8 + 3 + (isIPv6(&id.sockaddr) ? 16 : 4); // 0, 7, 19
    }
    return ofs;
}

GatewayListResponse::GatewayListResponse(
    const GatewayOperationRequest &request
)
    : GatewayOperationResponse(request)
{
}

void GatewayListResponse::ntoh()
{
    GatewayOperationResponse::ntoh();
    for (auto it(identities.begin()); it != identities.end(); it++) {
        it->gatewayId = NTOH8(it->gatewayId);
        sockaddrNtoh(&it->sockaddr);
    }
}

/**
 * IP v4 sizes:
 *  1   22 + 15 = 37
 *  2   22 + 2 * 15 = 52
 *  ..
 *  10   22 + 10 * 15 = 172
 *  18   22 + 18 * 15 = 292
 * IP v6 sizes:
 *  1   22 + 27 = 49
 *  2   22 + 2 * 27 = 76
 *  ..
 *  10   22 + 10 * 27 = 292
 * @param retBuf
 * @return
 */
size_t GatewayListResponse::serialize(
    unsigned char *retBuf
) const
{
    size_t ofs = GatewayOperationResponse::serialize(retBuf);   // 22
    if (retBuf) {
        for (auto it(identities.begin()); it != identities.end(); it++) {
            memmove(&retBuf[ofs], &it->gatewayId, sizeof(uint64_t));  // 8
            ofs += 8;
            ofs += serializeSocketAddress(&retBuf[ofs], &it->sockaddr);   // 0, 7, 19
        }
    } else {
        for (auto it(identities.begin()); it != identities.end(); it++) {
            ofs += 8;
            ofs += serializeSocketAddress(nullptr, &it->sockaddr);   // 0, 7, 19
        }
    }
    return ofs;
}

std::string GatewayListResponse::toJsonString() const {
    std::stringstream ss;
    ss << R"({"result": )" << GatewayOperationResponse::toJsonString()
       << ", \"gateways\": [";
    bool isFirst = true;
    for (uint32_t i = 0; i < response; i++) {
        if (isFirst)
            isFirst = false;
        else
            ss << ", ";
        ss << identities[i].toJsonString();
    }
    ss <<  "]}";
    return ss.str();
}

size_t GatewayListResponse::shortenList2Fit(
    size_t serializedSize
) {
    size_t r = this->serialize(nullptr);
    while(!identities.empty() && r > serializedSize) {
        identities.erase(identities.end() - 1);
        r = this->serialize(nullptr);
    }
    return r;
}

GatewayBinarySerialization::GatewayBinarySerialization(
    GatewayService *aSvc,
    int32_t aCode,
    uint64_t aAccessCode
)
    : GatewaySerialization(SKT_BINARY, aSvc, aCode, aAccessCode)
{

}

/**
 * Get size for serialized list
 * @param sz count ofg items
 * @return size in bytes
 */
static size_t getMaxGatewayListResponseSize(
    size_t sz
)
{
    return SIZE_OPERATION_RESPONSE + sz * (sizeof(uint64_t) + 19);
}

static size_t getListResponseSize(
    const std::vector<GatewayIdentity> &list
)
{
    size_t r = SIZE_OPERATION_RESPONSE;
    for (const auto & it : list) {
        r += sizeof(uint64_t) + serializeSocketAddress(nullptr, &it.sockaddr);
    }
    return r;
}

size_t GatewayBinarySerialization::query(
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
    ServiceMessage *pMsg = deserializeGateway((const unsigned char *) request, sz);
    if (!pMsg)
        return 0;   // unknown request
    if ((pMsg->code != code) || (pMsg->accessCode != accessCode)) {
#ifdef ENABLE_DEBUG
        std::cerr << ERR_ACCESS_DENIED
            << ": " << pMsg->code
            << "," << pMsg->accessCode
            << std::endl;
#endif
        auto r = new GatewayOperationResponse;
        r->code = ERR_CODE_ACCESS_DENIED;
        r->ntoh();
        r->serialize(retBuf);
        delete pMsg;
        return sizeof(GatewayOperationResponse);
    }

    ServiceMessage *r = nullptr;
    switch (request[0]) {
        case QUERY_GATEWAY_ADDR:   // request gateway identifier(with address) by network address. Return 0 if success
            {
                auto gr = (GatewayIdRequest *) pMsg;
                r = new GatewayGetResponse(*gr);
                memmove(&((GatewayGetResponse*) r)->response.sockaddr, &gr->id, sizeof(gr->id));
                svc->get(((GatewayGetResponse*) r)->response, ((GatewayGetResponse*) r)->response);
                break;
            }
        case QUERY_GATEWAY_ID:   // request gateway address (with identifier) by identifier. Return 0 if success
            {
                auto gr = (GatewayAddrRequest *) pMsg;
                r = new GatewayGetResponse(*gr);
                memmove(&((GatewayGetResponse*) r)->response.sockaddr, &gr->addr, sizeof(struct sockaddr));
                r->code = svc->get(((GatewayGetResponse*) r)->response, ((GatewayGetResponse*) r)->response);
                break;
            }
        case QUERY_GATEWAY_ASSIGN:   // assign (put) gateway address to the gateway by identifier
            {
                auto gr = (GatewayIdAddrRequest *) pMsg;
                r = new GatewayOperationResponse(*gr);
                int errCode = svc->put(gr->identity);
                ((GatewayOperationResponse *) r)->response = errCode;
                if (errCode == 0)
                    ((GatewayOperationResponse *) r)->size = 1;    // count of placed entries
                break;
            }
        case QUERY_GATEWAY_RM:   // Remove entry
            {
                auto gr = (GatewayIdAddrRequest *) pMsg;
                r = new GatewayOperationResponse(*gr);
                int errCode = svc->rm(gr->identity);
                ((GatewayOperationResponse *) r)->response = errCode;
                if (errCode == 0)
                    ((GatewayOperationResponse *) r)->size = 1;    // count of deleted entries
                break;
            }
        case QUERY_GATEWAY_LIST:   // List entries
        {
            auto gr = (GatewayOperationRequest *) pMsg;
            r = new GatewayListResponse(*gr);
            svc->list(((GatewayListResponse *) r)->identities, gr->offset, gr->size);
            size_t serSize = getListResponseSize(((GatewayListResponse *) r)->identities);
            if (serSize > retSize) {
                serSize = ((GatewayListResponse *) r)->shortenList2Fit(serSize);
                if (serSize > retSize) {
                    delete r;
                    r = nullptr;
                }
                break;
            }
            break;
        }
        case QUERY_GATEWAY_COUNT:   // count
        {
            auto gr = (GatewayOperationRequest *) pMsg;
            r = new GatewayOperationResponse(*gr);
            r->tag = gr->tag;
            r->code = CODE_OK;
            r->accessCode = gr->accessCode;
            ((GatewayOperationResponse *) r)->response = (uint32_t) svc->size();
            break;
        }
        case QUERY_GATEWAY_FORCE_SAVE:   // force save
            break;
        case QUERY_GATEWAY_CLOSE_RESOURCES:   // close resources
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
enum GatewayQueryTag validateGatewayQuery(
    const unsigned char *buffer,
    size_t size
)
{
    switch (buffer[0]) {
        case QUERY_GATEWAY_ADDR:   // request gateway identifier(with address) by network address.
            if (size < SIZE_GATEWAY_ID_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_ADDR;
        case QUERY_GATEWAY_ID:   // request gateway address (with identifier) by identifier.
            if (size < SIZE_DEVICE_ADDR_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_ID;
        case QUERY_GATEWAY_ASSIGN:   // assign (put) gateway address to the gateway by identifier
            if (size < SIZE_DEVICE_ADDR_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_ASSIGN;
        case QUERY_GATEWAY_RM:   // Remove entry
            if (size < SIZE_DEVICE_ADDR_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_RM;
        case QUERY_GATEWAY_LIST:   // List entries
            if (size < SIZE_OPERATION_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_LIST;
        case QUERY_GATEWAY_COUNT:   // list count
            if (size < SIZE_OPERATION_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_COUNT;
        case QUERY_GATEWAY_FORCE_SAVE:   // force save
            if (size < SIZE_OPERATION_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_FORCE_SAVE;
        case QUERY_GATEWAY_CLOSE_RESOURCES:   // close resources
            if (size < SIZE_OPERATION_REQUEST)
                return QUERY_GATEWAY_NONE;
            return QUERY_GATEWAY_CLOSE_RESOURCES;
    default:
            break;
    }
    return QUERY_GATEWAY_NONE;
}

/**
 * Return required size for response
 * @param buffer serialized request
 * @param size buffer size
 * @return size in bytes
 */
size_t responseSizeForGatewayRequest(
    const unsigned char *buffer,
    size_t size
)
{
    enum GatewayQueryTag tag = validateGatewayQuery(buffer, size);
    switch (tag) {
        case QUERY_GATEWAY_ADDR:    // request gateway identifier(with address) by network address.
        case QUERY_GATEWAY_ID:      // request gateway address (with identifier) by identifier.
            return SIZE_DEVICE_GET_ADDR_6_RESPONSE;    // IPv4: 28 IPv6: 40
        case QUERY_GATEWAY_LIST:    // List entries
            {
                GatewayOperationRequest lr(buffer, size);
                return getMaxGatewayListResponseSize(lr.size);
            }
        default:
            break;
    }
    return SIZE_OPERATION_RESPONSE; // GatewayOperationResponse
}

/**
 * Return request object or NULL if packet is invalid
 * @param buf buffer
 * @param sz buffer size
 * @return return NULL if packet is invalid
 */
ServiceMessage* deserializeGateway(
    const unsigned char *buf,
    size_t sz
)
{
    if (sz < SIZE_SERVICE_MESSAGE)
        return nullptr;
    ServiceMessage *r;
    switch (buf[0]) {
        case QUERY_GATEWAY_ADDR:   // request gateway identifier(with address) by network address. Return 0 if success
            if (sz < SIZE_GATEWAY_ID_REQUEST)
                return nullptr;
            r = new GatewayIdRequest(buf, sz);
            break;
        case QUERY_GATEWAY_ID:   // request gateway address (with identifier) by identifier. Return 0 if success
            if (sz < SIZE_DEVICE_ADDR_REQUEST)
                return nullptr;
            r = new GatewayAddrRequest(buf, sz);
            break;
        case QUERY_GATEWAY_ASSIGN:   // assign (put) gateway address to the gateway by identifier
            if (sz < SIZE_DEVICE_EUI_ADDR_REQUEST)
                return nullptr;
            r = new GatewayIdAddrRequest(buf, sz);
            break;
        case QUERY_GATEWAY_RM:   // Remove entry
            if (sz < SIZE_GATEWAY_ID_REQUEST)   // it can contain id only(no address)
                return nullptr;
            r = new GatewayIdAddrRequest(buf, sz);
            break;
        case QUERY_GATEWAY_LIST:   // List entries
            if (sz < SIZE_OPERATION_REQUEST)
                return nullptr;
            r = new GatewayOperationRequest(buf, sz);
            break;
        case QUERY_GATEWAY_COUNT:   // count
            if (sz < SIZE_OPERATION_REQUEST)
                return nullptr;
            r = new GatewayOperationRequest(buf, sz);
            break;
        case QUERY_GATEWAY_FORCE_SAVE:   // force save
            if (sz < SIZE_OPERATION_REQUEST)
                return nullptr;
            r = new GatewayOperationRequest(buf, sz);
            break;
        case QUERY_GATEWAY_CLOSE_RESOURCES:   // close resources
            if (sz < SIZE_OPERATION_REQUEST)
                return nullptr;
            r = new GatewayOperationRequest(buf, sz);
            break;
        default:
            r = nullptr;
    }
    if (r)
        r->ntoh();
    return r;
}

const char* gatewayTag2string(
    enum GatewayQueryTag value
)
{
    switch (value) {
        case QUERY_GATEWAY_ADDR:
            return "gw-address";
        case QUERY_GATEWAY_ID:
            return "gw-identifier";
        case QUERY_GATEWAY_LIST:
            return "gw-list";
        case QUERY_GATEWAY_COUNT:
            return "gw-count";
        case QUERY_GATEWAY_ASSIGN:
            return "gw-assign";
        case QUERY_GATEWAY_RM:
            return "gw-remove";
        case QUERY_GATEWAY_FORCE_SAVE:
            return "gw-save";
        case QUERY_GATEWAY_CLOSE_RESOURCES:
            return "gw-close";
        default:
            return "";
    }
}

static std::string GWCS("AILCPRSE");

const std::string &gatewayCommandSet()
{
    return GWCS;
}

/**
 * Check does it gateway tag in the buffer
 * @param buffer buffer to check
 * @param size buffer size
 * @return true
 */
bool isGatewayTag(
    const unsigned char *buffer,
    size_t size
)
{
    if (size == 0)
        return false;
    switch (buffer[0]) {
        case QUERY_GATEWAY_ADDR:
        case QUERY_GATEWAY_ID:
        case QUERY_GATEWAY_LIST:
        case QUERY_GATEWAY_COUNT:
        case QUERY_GATEWAY_ASSIGN:
        case QUERY_GATEWAY_RM:
        case QUERY_GATEWAY_FORCE_SAVE:
        case QUERY_GATEWAY_CLOSE_RESOURCES:
            return true;
        default:
            return false;
    }
}

GatewayQueryTag isGatewayTag(const char *tag) {
    if (!tag)
        return QUERY_GATEWAY_NONE;
    const std::string &cs = gatewayCommandSet();
    auto len = strlen(tag);
    if (len == 1) {
        if (cs.find(*tag) != std::string::npos) {
            return (GatewayQueryTag) *tag;
        }
    } else {
        for (auto it : cs) {
            if (strcmp(gatewayTag2string((GatewayQueryTag) it), tag) == 0)
                return (GatewayQueryTag) it;
        }
    }
    return QUERY_GATEWAY_NONE;
}
