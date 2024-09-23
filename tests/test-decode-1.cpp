#include <string>
#include <iostream>
#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/helper/aes-helper.h"
#include "lorawan/proto/gw/basic-udp.h"

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
    KEY128 appSKey("D72C78758CDCCABF55EE4A778D16EF67");
    std::string s = decodePayload(hex2string("890b049d"), appSKey, DEVADDR(0x7e6ae2), 226, LORAWAN_UPLINK);
    std::cout << hexString(s) << std::endl;
    if (s != hex2string("aa11f508"))
        return 1;
}
