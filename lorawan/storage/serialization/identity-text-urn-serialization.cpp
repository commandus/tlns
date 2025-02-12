#include <string>
#include <sstream>

#include "lorawan/storage/serialization/identity-text-urn-serialization.h"
#include "lorawan/storage/serialization/urn-helper.h"

#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-string.h"

#ifdef ESP_PLATFORM
#include "platform-defs.h"
#endif

IdentityTextURNSerialization::IdentityTextURNSerialization(
    IdentityService* aSvc,
    int32_t aCode,
    uint64_t aAccessCode
)
    : IdentitySerialization(SKT_TEXT_JSON, aSvc, aCode, aAccessCode)
{

}

size_t IdentityTextURNSerialization::query(
    unsigned char* retBuf,
    size_t retSize,
    const unsigned char* request,
    size_t sz
)
{
    if (!svc)
        return 0;
    LorawanIdentificationURN urn(std::string((const char*) request, sz));
    switch (urn.command) {
        case 'A':
            // request identifier(with address) by LoRaWAN network address
        {
            if (urn.networkIdentity.value.devaddr.empty()) {
                // eui
                int r = svc->getNetworkIdentity(urn.networkIdentity, urn.networkIdentity.value.devid.id.devEUI);
                return returnURN(retBuf, retSize, urn, r);
            } else {
                // addr
                int r = svc->get(urn.networkIdentity.value.devid, urn.networkIdentity.value.devaddr);
                return returnURN(retBuf, retSize, urn, r);
            }
        }
            break;
        case 'I':
            // request address (with identifier) by identifier
        {
            int r = svc->get(urn.networkIdentity.value.devid, urn.networkIdentity.value.devaddr);
            return returnURN(retBuf, retSize, urn, r);
        }
            break;
        case 'L': {
            // request list
            std::vector<NETWORKIDENTITY> nis;
            int r = svc->list(nis, urn.offset, urn.size);
            std::stringstream ss;
            if (r == CODE_OK) {
                bool isFirst = true;
                for (auto &ni: nis) {
                    if (isFirst)
                        isFirst = false;
                    else
                        ss << ';';
                    LorawanIdentificationURN u;
                    u.networkIdentity = ni;
                    ss << u.toString();
                }

            }
            return returnStr(retBuf, retSize, ss.str(), r);
        }
        case 'c': {
            // count
            auto r = svc->size();
            return returnStr(retBuf, retSize, std::to_string(r), 0);
        }
        case 'n':
            // next
        {
            auto r = svc->next(urn.networkIdentity);
            return returnURN(retBuf, retSize, urn, r);
        }
        case 'p':
            // assign
        {
            auto r = svc->put(urn.networkIdentity.value.devaddr, urn.networkIdentity.value.devid);
            return returnStr(retBuf, retSize, std::to_string(r), 0);
        }
        case 'r':
            // remove entry
        {
            auto r = svc->rm(urn.networkIdentity.value.devaddr);
            return returnStr(retBuf, retSize, std::to_string(r), 0);
        }
        case 's':
            // force save
            return returnStr(retBuf, retSize, std::to_string(CODE_OK), 0);
        case 'e':
            // close resources
            return returnStr(retBuf, retSize, std::to_string(CODE_OK), 0);
        default:
            return 0;
    }
        return 0;
}
