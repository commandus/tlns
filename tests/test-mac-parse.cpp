#include <string>
#include <iostream>
#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/aes-helper.h"
#include "lorawan/lorawan-mac.h"
#include "lorawan/lorawan-error.h"

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

static std::string hexMacSequences[] {
    "029a5c9a1b",
    "029a5c9a5cb63627581cc91c",
    "02e7703191107ea2e9053839",
};

int main(int argc, char **argv) {
    for (auto &s : hexMacSequences) {
        std::cout << s << std::endl;
        MacPtr macs(hex2string(s).c_str(), 12);
        std::cout << macs.toJSONString() << std::endl;
        if (macs.errorcode < 0) {
            if (macs.errorcode == ERR_CODE_MAC_UNKNOWN_EXTENSION)
                std::cerr << "Has proprietary command(s)"<< std::endl;
            else {
                std::cerr << "Error " << macs.errorcode << std::endl;
                return macs.errorcode;
            }
        }
    }
}
