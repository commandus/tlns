#include <sstream>
#include <cstring>
#include "tlns-cli-helper.h"

static bool split3(
    std::string& retParam1,
    std::string& retParam2,
    std::string& retParam3,
    const std::string& value
)
{
    size_t pos1 = value.find_first_of(':');
    if (pos1 == std::string::npos)
        return false;
    size_t pos2 = value.find_last_of(':');
    if (pos2 == pos1) {
        retParam1 = value.substr(0, pos1);
        retParam2 = value.substr(pos1 + 1);
        retParam3 = retParam2;
        return true;
    }
    retParam1 = value.substr(0, pos1);
    retParam2 = value.substr(pos1 + 1, pos2 - pos1 - 1);
    retParam3 = value.substr(pos2 + 1);
    return true;
}

/**
 * Split @param address e.g. FILE:CLASS to @param retFile, @param retIdentityClass, @param retGatewayClass
 */
bool splitFileClass(
    std::string& retFile,
    std::string& retIdentityClass,
    std::string& retGatewayClass,
    const std::string& value
)
{
    return split3(retFile, retIdentityClass, retGatewayClass, value);
}

/**
 * Split @param value e.g. "user:password:topic" to @param retUser,  @param retPassword and @param retTopic
 */
bool splitUserPasswordTopic(
    std::string& retUser,
    std::string& retPassword,
    std::string& retTopic,
    const std::string& value
)
{
    return split3(retUser, retPassword, retTopic, value);
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
