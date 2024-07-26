#include <sstream>
#include <cstring>
#include "tlns-cli-helper.h"

/**
 * Split @param address e.g. FILE:CLASS to @param retFile and @param retClass
 */
bool splitFileClass(
    std::string& retFile,
    std::string& retIdentityClass,
    std::string& retGatewayClass,
    const std::string& value
)
{
    size_t pos1 = value.find_first_of(':');
    if (pos1 == std::string::npos)
        return false;
    size_t pos2 = value.find_last_of(':');
    if (pos2 == pos1) {
        retFile = value.substr(0, pos1);
        retIdentityClass = value.substr(pos1 + 1);
        retGatewayClass = retIdentityClass;
        return true;
    }
    retFile = value.substr(0, pos1);
    retIdentityClass = value.substr(pos1 + 1, pos2 - pos1 - 1);
    retGatewayClass = value.substr(pos2 + 1);
    return true;
}

bool readNetId(
    NETID &retVal,
    const std::string &value
)
{
    auto p = value.find(':');
    if (p != std::string::npos) {
        retVal.set(
            (uint8_t) strtoul(value.substr(0, p - 1).c_str(), nullptr, 16),
            (uint32_t) strtoul(value.substr(p + 1).c_str(), nullptr, 16)
        );
    } else
        retVal.set(strtoul(value.c_str(), nullptr, 16));
    return true;
}
