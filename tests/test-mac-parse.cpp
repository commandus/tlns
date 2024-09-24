#include <string>
#include <iostream>
#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/aes-helper.h"
#include "lorawan/lorawan-mac.h"

static std::string decodePayload(
    const std::string &payload,
    const KEY128 &appSKey,
    const DEVADDR &addr,
    int fcnt,
    int direction
)
{
    decryptPayloadString(payload, fcnt, direction & 1, addr, appSKey);
    return payload;
}

int main(int argc, char **argv) {
    std::string s("029a5c9a5cb63627581cc91c");
    std::cout << s << std::endl;
    MacPtr macs(hex2string(s).c_str(), 12, true);
    std::cout << macs.toJSONString() << std::endl;

    s = ("029a5c9a1b");
    std::cout << s << std::endl;
    macs.parse(hex2string(s).c_str(), 12);
    std::cout << macs.toJSONString() << std::endl;
}
