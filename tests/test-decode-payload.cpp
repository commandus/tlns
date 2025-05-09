#include <string>
#include <iostream>
#include <cassert>
#include <iomanip>
#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/aes-helper.h"
#include "lorawan/lorawan-mic.h"

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
    std::string s = decodePayload(hex2string("890b049d"), KEY128 ("D72C78758CDCCABF55EE4A778D16EF67"), DEVADDR(0x7e6ae2), 226, LORAWAN_UPLINK);
    std::cout << hexString(s) << std::endl;
    assert(s == hex2string("aa11f508"));

    s = decodePayload(hex2string("6B69636B6173732D776F7A6E69616B"), KEY128("d72c78758cdccabf55ee4a778d16ef67"), DEVADDR(0x007E6AE1), 0, LORAWAN_DOWNLINK);
    std::cout << hexString(s) << std::endl;
    assert(s == hex2string("75D0A12554979895ABA8AAB2487C7D"));
    std::cout << "Uplink & downlink messages decoded successfully" << std::endl;

    // test MIC
    s = hex2string("60E16A7E00000000026B69636B6173732D776F7A6E69616B");
    uint32_t mic = calculateMICFrmPayload((const unsigned char *) s.c_str(), s.size(), 0,
        LORAWAN_DOWNLINK, DEVADDR(0x007E6AE1), KEY128("15b1d0efa463dfbe3d11181e1ec7da85"));
    std::cout << std::hex << std::setfill('0') << std::setw(8) << mic << std::endl;
    assert(mic == 0x9B1777C2);

    // test MIC
    s = hex2string("60E26A7E00000000026B69636B6173732D776F7A6E69616B");
    mic = calculateMICFrmPayload((const unsigned char *) s.c_str(), s.size(), 0,
        LORAWAN_DOWNLINK, DEVADDR(0x007E6AE2), KEY128("15b1d0efa463dfbe3d11181e1ec7da85"));
    std::cout << std::hex << std::setfill('0') << std::setw(8) << mic << std::endl;
    assert(mic == 0xC2A0490F);

}
