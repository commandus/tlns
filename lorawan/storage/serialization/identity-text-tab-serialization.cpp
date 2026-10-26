#include <sstream>

#include "lorawan/storage/serialization/identity-text-tab-serialization.h"
#include "lorawan/storage/serialization/json-helper.h"
#include "lorawan/lorawan-conv.h"

#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"

#ifdef ESP_PLATFORM
#include "platform-defs.h"
#endif

#ifdef ENABLE_DEBUG
#include <iostream>
#include "lorawan-msg.h"
#endif

IdentityTextTabSerialization::IdentityTextTabSerialization(
    IdentityService* aSvc,
    int32_t aCode,
    uint64_t aAccessCode
)
    : IdentitySerialization(SKT_TEXT_TAB, aSvc, aCode, aAccessCode)
{

}

size_t IdentityTextTabSerialization::query(
    unsigned char* retBuf,
    size_t retSize,
    const unsigned char* request,
    size_t sz
)
{
    if (!svc)
        return 0;
    // Implement check credentials here
    std::stringstream ss((const char *)request);
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> args(begin, end);
    auto argsCount = args.size();
    if (!argsCount)
        return 0;
    std::string tag = args[0];
    char t = tag[0];
    switch (t) {
        case 'a':
            // request identifier(with address) by LoRaWAN network address
        {
            // get address
            if (argsCount < 2)
                break;
            bool isAddress = isDEVADDRString(args[1]);
            bool isEUI = isDEVEUIString(args[1]);
            if (isEUI) {
                // eui
                DEVEUI devEUI;
                string2DEVEUI(devEUI, args[1]);
                NETWORKIDENTITY nid;
                int r = svc->getNetworkIdentity(nid, devEUI);
                if (r == CODE_OK) {
                    return retStr(retBuf, retSize, nid.toJsonString());
                } else {
                    return retStatusCode(retBuf, retSize, r);
                }
            } else {
                if (!isAddress)
                    break;
                // addr
                DEVADDR a;
                string2DEVADDR(a, args[1]);
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
            if (argsCount < 2)
                break;
            if (!isDEVADDRString(args[1]))
                break;
            DEVADDR a;
            string2DEVADDR(a, args[1]);
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
            if (argsCount > 1) {
                if (isDec(args[1])) {
                    offset = (uint32_t) strtol(args[1].c_str(), nullptr, 10);
                    if (argsCount > 2) {
                        if (isDec(args[2]))
                            size = (uint8_t) strtol(args[2].c_str(), nullptr, 10);
                    }
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
            if (argsCount < 2)
                break;  // at least address is required
            // <addr> <activation> <class> <deveui> <nwkSKey> <appSKey> <version> <appeui> <appKey> <nwkKey> <devNonce> <joinNonce> <name>
            if (!isDEVADDRString(args[1]))
                break;
            DEVADDR deviceAddr;
            string2DEVADDR(deviceAddr, args[1]);
            DEVICEID deviceId;
            if (argsCount > 2) {
                deviceId.id.activation = string2activation(args[2]);
                if (argsCount > 3) {
                    deviceId.setClass(string2deviceclass(args[3]));
                    if (argsCount > 4) {
                        string2DEVEUI(deviceId.id.devEUI, args[4]);
                        if (argsCount > 5) {
                            string2KEY(deviceId.id.nwkSKey, args[5]);
                            if (argsCount > 6) {
                                string2KEY(deviceId.id.appSKey, args[6]);
                                if (argsCount > 7) {
                                    string2KEY(deviceId.id.appSKey,args[7]);
                                    if (argsCount > 8) {
                                        deviceId.id.version = string2LORAWAN_VERSION(args[8]);
                                        if (argsCount > 9) {
                                            string2DEVEUI(deviceId.id.appEUI, args[9]);
                                            if (argsCount > 10) {
                                                string2KEY(deviceId.id.appKey, args[10]);
                                                if (argsCount > 11) {
                                                    string2KEY(deviceId.id.nwkKey, args[11]);
                                                    if (argsCount > 12) {
                                                        deviceId.id.devNonce = string2DEVNONCE(args[12]);
                                                        if (argsCount > 13) {
                                                            string2JOINNONCE(deviceId.id.joinNonce, args[13]);
                                                            if (argsCount > 14) {
                                                                string2DEVICENAME(deviceId.id.name, args[14].c_str());
                                                            }
                                                        }
                                                    }
                                                }

                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            auto r = svc->put(deviceAddr, deviceId);
            return retStatusCode(retBuf, retSize, r);
        }
        case 'r':
            // remove entry
        {
            // get address
            if (argsCount < 2)
                break;
            if (!isDEVADDRString(args[1]))
                break;
            DEVADDR deviceAddr;
            string2DEVADDR(deviceAddr, args[1]);
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
