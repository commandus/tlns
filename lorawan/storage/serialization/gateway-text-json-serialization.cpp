#include <sstream>

#include "lorawan/storage/serialization/gateway-text-json-serialization.h"
#include "lorawan/helper/ip-address.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/storage/serialization/json-helper.h"

#ifdef ESP_PLATFORM
#include "platform-defs.h"
#endif

#ifdef ENABLE_DEBUG
#include <iostream>
#include "lorawan/lorawan-msg.h"
#endif

GatewayTextJSONSerialization::GatewayTextJSONSerialization(
    GatewayService *aSvc,
    int32_t aCode,
    uint64_t aAccessCode
)
    : GatewaySerialization(SKT_TEXT_JSON, aSvc, aCode, aAccessCode)
{

}

size_t GatewayTextJSONSerialization::query(
    unsigned char *retBuf,
    size_t retSize,
    const unsigned char *request,
    size_t sz
)
{
    if (!svc)
        return 0;
    nlohmann::json js;
    try {
        js = nlohmann::json::parse(request, request + sz);
    } catch (nlohmann::json::exception &) {
        return 0;
    }
    if (!js.is_object())
        return 0;
    if (!checkCredentials(js, code, accessCode)) {
#ifdef ENABLE_DEBUG
        std::cerr << ERR_ACCESS_DENIED << std::endl;
#endif
        return 0;
    }

    if (!js.contains("tag"))
        return 0;
    auto jTag = js["tag"];
    if (!jTag.is_string())
        return 0;
    std::string tag = jTag;
    if (tag.empty())
        return 0;
    char t = tag[0];
    switch (t) {
        case 'A':
            // request gateway identifier(with address) by IP address
        {
            // get address
            if (!js.contains("addr"))
                return retStatusCode(retBuf, retSize, ERR_CODE_GATEWAY_NOT_FOUND);
            auto jAddr = js["addr"];
            if (!jAddr.is_string())
                return retStatusCode(retBuf, retSize, ERR_CODE_GATEWAY_NOT_FOUND);
            GatewayIdentity gi;
            string2sockaddr(&gi.sockaddr, jAddr);
            int r = svc->get(gi, gi);
            if (r)
                return retStatusCode(retBuf, retSize, r);
            return retStr(retBuf, retSize, gi.toJsonString());
        }
            break;
        case 'I':
            // request address (with identifier) by identifier
        {
            // get address
            if (!js.contains("gwid"))
                return retStatusCode(retBuf, retSize, ERR_CODE_GATEWAY_NOT_FOUND);
            auto jId = js["gwid"];
            if (!jId.is_string())
                return retStatusCode(retBuf, retSize, ERR_CODE_GATEWAY_NOT_FOUND);
            GatewayIdentity gi;
            gi.gatewayId = string2gatewayId(jId);
            int r = svc->get(gi, gi);
            if (r)
                return retStatusCode(retBuf, retSize, r);
            return retStr(retBuf, retSize, gi.toJsonString());
        }
            break;
        case 'L': {
            // request list
            uint32_t offset = 0;
            uint8_t size = 10;
            if (js.contains("offset")) {
                auto jOffset = js["offset"];
                if (jOffset.is_number()) {
                    offset = jOffset;
                }
            }
            if (js.contains("size")) {
                auto jSize = js["size"];
                if (jSize.is_number()) {
                    size = jSize;
                }
            }
            std::vector<GatewayIdentity> nis;
            int r = svc->list(nis, offset, size);
            if (r == CODE_OK) {
                std::stringstream ss;
                bool isFirst = true;
                ss << "[";
                for (auto &ni: nis) {
                    if (isFirst)
                        isFirst = false;
                    else
                        ss << ", ";
                    ss << ni.toJsonString();
                }
                ss << "]";
                return retStr(retBuf, retSize, ss.str());
            } else
                return retStatusCode(retBuf, retSize, r);
        }
        case 'C': {
            // count
            auto r = svc->size();
            return retStr(retBuf, retSize, std::to_string(r));
        }
        case 'P':
            // assign
        {
            std::string addr;
            if (js.contains("addr")) {
                auto jAddr = js["addr"];
                if (jAddr.is_string())
                    addr = jAddr;
            }
            std::string gwid;
            if (js.contains("gwid")) {
                auto jGwid = js["gwid"];
                if (jGwid.is_string())
                    gwid = jGwid;
            }
            GatewayIdentity gi;
            if (!addr.empty())
                string2sockaddr(&gi.sockaddr, addr);
            if (!gwid.empty())
                gi.gatewayId = string2gatewayId(gwid);
            auto r = svc->put(gi);
            return retStatusCode(retBuf, retSize, r);
        }
        case 'R':
            // remove entry
        {
            std::string addr;
            if (js.contains("addr")) {
                auto jAddr = js["addr"];
                if (jAddr.is_string())
                    addr = jAddr;
            }
            std::string gwid;
            if (js.contains("gwid")) {
                auto jGwid = js["gwid"];
                if (jGwid.is_string())
                    gwid = jGwid;
            }
            GatewayIdentity gi;
            if (!addr.empty())
                string2sockaddr(&gi.sockaddr, addr);
            if (!gwid.empty())
                gi.gatewayId = string2gatewayId(gwid);
            auto r = svc->rm(gi);
            return retStatusCode(retBuf, retSize, r);
        }
        case 's':
            // force save
            return retStatusCode(retBuf, retSize, CODE_OK);
        case 'e':
            // close resources
            return retStatusCode(retBuf, retSize, CODE_OK);
        default:
            return 0;
    }
    return 0;
}
