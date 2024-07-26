/*
 * Command line helper routines
 * @file cli-helper.h
 */
#ifndef TLNS_TLNS_CLI_HELPER_H
#define TLNS_TLNS_CLI_HELPER_H     1

#include <string>
#include <vector>

#include "lorawan/lorawan-types.h"

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
