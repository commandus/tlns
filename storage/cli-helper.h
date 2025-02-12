/*
 * Command line helper routines
 * @file cli-helper.h
 */
#ifndef CLI_HELPER_H
#define CLI_HELPER_H     1

#include <string>
#include <vector>

#include "lorawan/lorawan-types.h"
#include "lorawan/storage/serialization/identity-serialization.h"
#include "lorawan/storage/serialization/identity-binary-serialization.h"
#include "lorawan/storage/serialization/gateway-serialization.h"

class DeviceOrGatewayIdentity {
public:
    bool hasDevice;
    bool hasGateway;
    GatewayIdentity gid;
    NETWORKIDENTITY nid;
    DeviceOrGatewayIdentity()
        : hasDevice(false), hasGateway(false)
    {};
};

std::string listCommands();

std::string listPlugins();

std::string shortCommandList(char delimiter);

std::string commandLongName(int tag);

/**
 * Merge address and identifiers
 * @param query Each item has address or identifier
 * @return true if success
 */
bool mergeIdAddress(
    std::vector<DeviceOrGatewayIdentity> &query
);

/**
 * Split @param address e.g. FILE:CLASS to @param retFile and @param retClass
 */
bool splitFileClass(
    std::string& retFile,
    std::string& retIdentityClass,
    std::string& retGatewayClass,
    const std::string& value
);

/**
 * Read hexadecimal <network-id> or <net-type>:<net-id>.
 * @param retVal return value
 * @param value string to read
 * @return true
 */
bool readNetId(
    NETID &retVal,
    const std::string &value
);

#endif
