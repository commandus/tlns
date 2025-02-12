#include "lorawan/storage/serialization/json-helper.h"

size_t retJs(
    unsigned char* retBuf,
    size_t retSize,
    const nlohmann::json &js
)
{
    std::string s = js.dump();
    auto r = s.size();
    if (r <= retSize) {
        memmove(retBuf, s.c_str(), s.size());
    } else
        r = 0;
    return r;
}

 size_t retStr(
    unsigned char* retBuf,
    size_t retSize,
    const std::string &s
)
{
    auto r = s.size();
    if (r <= retSize) {
        memmove(retBuf, s.c_str(), s.size());
    } else
        r = 0;
    return r;
}

size_t retStatusCode(
    unsigned char* retBuf,
    size_t retSize,
    int errCode
)
{
    nlohmann::json js;
    js["code"] = errCode;
    return retJs(retBuf, retSize, js);
}

bool checkCredentials(
    const nlohmann::json &js,
    int32_t code,
    uint64_t accessCode
)
{
    if (!js.contains("code"))
        return false;
    auto &jCode = js["code"];
    if (jCode.is_number()) {
        if (code != (int32_t ) jCode)
            return false;
    } else {
        if (jCode.is_string()) {
            std::string s = jCode;
            if (!((code == (int32_t) strtoll(s.c_str(), nullptr, 16))
                || (code == (int32_t) strtoll(s.c_str(), nullptr, 10))
            ))
                return false;
        } else
            return false;
    }

    if (!js.contains("accessCode"))
        return false;
    auto &jAccessCode = js["accessCode"];
    if (jAccessCode.is_number()) {
        if (accessCode != (uint64_t) jAccessCode)
            return false;
    } else {
        if (jAccessCode.is_string()) {
            std::string s = jAccessCode;
            if (!((accessCode == (uint64_t) strtoull(s.c_str(), nullptr, 16))
                || (accessCode == (uint64_t) strtoull(s.c_str(), nullptr, 10))
            ))
                return false;
        } else
            return false;
    }
    return true;
}
