#include <string>
#include <iostream>
#include <cassert>
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
    std::string s = decodePayload(hex2string("890b049d"), KEY128 ("d72c78758cdccabf55ee4a778d16ef67"),
        DEVADDR(0x7e6ae2), 226, LORAWAN_UPLINK);
    std::cout << hexString(s) << std::endl;
    assert(s == hex2string("aa11f508"));

    s = decodePayload(hex2string("6b69636b6173732d776f7a6e69616b"), KEY128("d72c78758cdccabf55ee4a778d16ef67"),
        DEVADDR(0x007e6ae1), 0, LORAWAN_DOWNLINK);
    std::cout << hexString(s) << std::endl;
    assert(s == hex2string("75d0a12554979895aba8aab2487c7d"));
    std::cout << "Uplink & downlink messages decoded successfully" << std::endl;

    // test MIC
    s = hex2string("60E16A7E00000000026B69636B6173732D776F7A6E69616B");
    uint32_t mic = calculateMICFrmPayload((const unsigned char *) s.c_str(), s.size(), 0,
        LORAWAN_DOWNLINK, DEVADDR(0x007E6AE1), KEY128("15b1d0efa463dfbe3d11181e1ec7da85"));
    std::cout << MIC2String(mic) << std::endl;
    assert(mic == 0x9b1777c2);

    // test MIC
    s = hex2string("60e26a7e00000000026b69636b6173732d776f7a6e69616b");
    mic = calculateMICFrmPayload((const unsigned char *) s.c_str(), s.size(), 0,
        LORAWAN_DOWNLINK, DEVADDR(0x007E6AE2), KEY128("15b1d0efa463dfbe3d11181e1ec7da85"));
    std::cout << MIC2String(mic) << std::endl;
    assert(mic == 0xc2a0490f);

    // test MIC, payload 607b950a00000000026b69636b6173732d776f7a6e69616bd0955d84
    s = hex2string("607b950a00000000026b69636b6173732d776f7a6e69616b");
    mic = calculateMICFrmPayload((const unsigned char *) s.c_str(), s.size(), 0,
        LORAWAN_DOWNLINK, DEVADDR(0x000A957B), KEY128("2b7e151628aed2a6abf7158809cf4f3c"));
    std::cout << MIC2String(mic) << std::endl;
    assert(mic == 0xd0955d84);
}
