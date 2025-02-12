#include <sstream>

#include "nlohmann/json.hpp"

#include "lorawan/storage/serialization/identity-text-json-serialization.h"
#include "lorawan/storage/serialization/json-helper.h"
#include "lorawan/lorawan-conv.h"

#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"

#ifdef ESP_PLATFORM
#include "platform-defs.h"
#endif

IdentityTextJSONSerialization::IdentityTextJSONSerialization(
    IdentityService* aSvc,
    int32_t aCode,
    uint64_t aAccessCode
)
    : IdentitySerialization(SKT_TEXT_JSON, aSvc, aCode, aAccessCode)
{

}

size_t IdentityTextJSONSerialization::query(
    unsigned char* retBuf,
    size_t retSize,
    const unsigned char* request,
    size_t sz
)
{
    if (!svc)
        return 0;
    nlohmann::json js;
    try {
        js = nlohmann::json::parse(request, request + sz);
    } catch (nlohmann::json::exception &) {
#ifdef ENABLE_DEBUG
        std::cerr << ERR_ACCESS_DENIED << " " << exception.what() << std::endl;
#endif
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
        case 'a':
            // request identifier(with address) by LoRaWAN network address
        {
            // get address
            std::string addr;
            if (js.contains("addr")) {
                auto jAddr = js["addr"];
                if (jAddr.is_string()) {
                    addr = jAddr;
                }
            }
            // or eui
            std::string eui;
            if (js.contains("eui")) {
                auto jEui = js["eui"];
                if (jEui.is_string()) {
                    eui = jEui;
                }
            }
            if (addr.empty()) {
                // eui
                DEVEUI devEUI;
                string2DEVEUI(devEUI, eui);
                NETWORKIDENTITY nid;
                int r = svc->getNetworkIdentity(nid, devEUI);
                if (r == CODE_OK) {
                    return retStr(retBuf, retSize, nid.toJsonString());
                } else {
                    return retStatusCode(retBuf, retSize, r);
                }
            } else {
                // addr
                DEVADDR a;
                string2DEVADDR(a, addr);
                DEVICEID did;
                int r = svc->get(did, a);
                if (r == CODE_OK)
                    return retStr(retBuf, retSize, did.toJsonString());
                else
                    return retStatusCode(retBuf, retSize, r);
            }
        }
            break;
        case 'i':
            // request address (with identifier) by identifier
        {
            std::string addr;
            if (js.contains("addr")) {
                auto jAddr = js["addr"];
                if (jAddr.is_string()) {
                    addr = jAddr;
                }
            }
            DEVADDR a;
            string2DEVADDR(a, addr);
            DEVICEID did;
            int r = svc->get(did, a);
            if (r == CODE_OK)
                return retStr(retBuf, retSize, did.toJsonString());
            else
                return retStatusCode(retBuf, retSize, r);
        }
            break;
        case 'l': {
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
            std::vector<NETWORKIDENTITY> nis;
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
        case 'c': {
            // count
            auto r = svc->size();
            return retStr(retBuf, retSize, std::to_string(r));
        }
        case 'n':
            // next
        {
            NETWORKIDENTITY ni;
            auto r = svc->next(ni);
            if (r)
                return retStatusCode(retBuf, retSize, r);
            else
                return retStr(retBuf, retSize, ni.toJsonString());
        }
        case 'p':
            // assign
        {
            DEVADDR deviceAddr;
            if (!js.contains("addr"))
                return 0;
            auto jAddr = js["addr"];
            if (!jAddr.is_string())
                return 0;
            string2DEVADDR(deviceAddr, jAddr);

            DEVICEID deviceId;
            if (js.contains("activation")) {
                auto jsv = js["activation"];
                if (jsv.is_string())
                    deviceId.id.activation = string2activation(jsv);
            }

            if (js.contains("class")) {
                auto jsv = js["class"];
                if (jsv.is_string())
                    deviceId.setClass(string2deviceclass(jsv));
            }

            if (js.contains("deveui")) {
                auto jsv = js["deveui"];
                if (jsv.is_string())
                    string2DEVEUI(deviceId.id.devEUI, jsv);
            }

            if (js.contains("nwkSKey")) {
                auto jsv = js["nwkSKey"];
                if (jsv.is_string())
                    string2KEY(deviceId.id.nwkSKey, jsv);
            }
            if (js.contains("appSKey")) {
                auto jsv = js["appSKey"];
                if (jsv.is_string())
                    string2KEY(deviceId.id.appSKey, jsv);
            }

            if (js.contains("version")) {
                auto jsv = js["version"];
                if (jsv.is_string())
                    deviceId.id.version = string2LORAWAN_VERSION(jsv);
            }

            if (js.contains("appeui")) {
                auto jsv = js["appeui"];
                if (jsv.is_string())
                    string2DEVEUI(deviceId.id.appEUI, jsv);
            }

            if (js.contains("appKey")) {
                auto jsv = js["appKey"];
                if (jsv.is_string())
                    string2KEY(deviceId.id.appKey, jsv);
            }

            if (js.contains("nwkKey")) {
                auto jsv = js["nwkKey"];
                if (jsv.is_string())
                    string2KEY(deviceId.id.nwkKey, jsv);
            }

            if (js.contains("devNonce")) {
                auto jsv = js["devNonce"];
                if (jsv.is_string())
                    deviceId.id.devNonce = string2DEVNONCE(jsv);
            }

            if (js.contains("joinNonce")) {
                auto jsv = js["joinNonce"];
                if (jsv.is_string())
                    string2JOINNONCE(deviceId.id.joinNonce, jsv);
            }

            if (js.contains("name")) {
                auto jsv = js["name"];
                if (jsv.is_string()) {
                    std::string s(jsv);
                    string2DEVICENAME(deviceId.id.name, s.c_str());
                }
            }

            auto r = svc->put(deviceAddr, deviceId);
            return retStatusCode(retBuf, retSize, r);
        }
        case 'r':
            // remove entry
        {
            // get address
            DEVADDR deviceAddr;
            std::string addr;
            if (!js.contains("addr"))
                return retStatusCode(retBuf, retSize, ERR_CODE_DEVICE_ADDRESS_NOTFOUND);
            auto jAddr = js["addr"];
            if (!jAddr.is_string())
                return retStatusCode(retBuf, retSize, ERR_CODE_DEVICE_ADDRESS_NOTFOUND);
            string2DEVADDR(deviceAddr, jAddr);
            auto r = svc->rm(deviceAddr);
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
