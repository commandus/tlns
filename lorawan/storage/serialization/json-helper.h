#ifndef LORAWAN_STORAGE_JSON_HELPER_H
#define LORAWAN_STORAGE_JSON_HELPER_H

#include <nlohmann/json.hpp>

size_t retJs(
    unsigned char* retBuf,
    size_t retSize,
    const nlohmann::json &js
);

size_t retStr(
    unsigned char* retBuf,
    size_t retSize,
    const std::string &s
);

size_t retStatusCode(
    unsigned char* retBuf,
    size_t retSize,
    int errCode
);

bool checkCredentials(
    const nlohmann::json &js,
    int32_t code,
    uint64_t accessCode
);

#endif
