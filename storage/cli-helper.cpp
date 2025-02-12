#include <sstream>
#include <cstring>
#include "cli-helper.h"
#include "lorawan/storage/serialization/identity-binary-serialization.h"
#include "lorawan/storage/serialization/gateway-binary-serialization.h"

std::string listCommands() {
    std::stringstream ss;
    const std::string &cs = identityCommandSet();
    for (auto i = 0; i < cs.size(); i++) {
        auto c = cs[i];
        ss << "  " << c << "\t" << identityTag2string((enum IdentityQueryTag) c);
        if (c == QUERY_IDENTITY_ADDR)
            ss << " (default)";
        ss << "\n";
    }
    for (auto i = 0; i < cs.size(); i++) {
        auto c = gatewayCommandSet()[i];
        ss << "  " << c << "\t" << gatewayTag2string((enum GatewayQueryTag) c);
        ss << "\n";
    }
    return ss.str();
}

std::string listPlugins() {
    std::stringstream ss;
    ss << "mem|gen|json";
#ifdef ENABLE_SQLITE
    ss << "|sqlite";
#endif
    ss << "|<file-path:identity-class::gateway-class";
    return ss.str();
}

std::string shortCommandList(char delimiter) {
    std::stringstream ss;
    ss << "command: ";
    const std::string &cs = identityCommandSet();
    for (auto i = 0; i < cs.size(); i++) {
        ss << cs[i] << delimiter;
    }
    for (auto i = 0; i < cs.size(); i++) {
        ss << cs[i] << delimiter;
    }
    ss << cs[cs.size() - 1];
    ss << ", gateway id- 16, device id- 8 hex digits";
    return ss.str();
}

std::string commandLongName(int tag)
{
    std::string r = identityTag2string((enum IdentityQueryTag) tag);
    if (r.empty())
        r = gatewayTag2string((enum GatewayQueryTag) tag);
    return r;
}

/**
 * Merge address and identifiers
 * @param query Each item has address or identifier
 * @return true if success
 */
bool mergeIdAddress(
    std::vector<DeviceOrGatewayIdentity> &query
)
{
    auto pairSize = query.size() / 2;
    for (int i = 0; i < pairSize; i++) {
        if (query[i * 2].gid.gatewayId) {
            query[i].gid.gatewayId = query[i * 2].gid.gatewayId;
            memmove(&query[i].gid.sockaddr, &query[(i * 2) + 1].gid.sockaddr, sizeof(struct sockaddr));
        } else {
            memmove(&query[i].gid.sockaddr, &query[(i * 2)].gid.sockaddr, sizeof(struct sockaddr));
            query[i].gid.gatewayId = query[(i * 2) + 1].gid.gatewayId;
        }
    }
    query.resize(pairSize);
    return true;
}

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
