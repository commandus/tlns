#include <string>
#include <iostream>
#include <cassert>
#include "lorawan/proto/payload2device/payload2device-parser.h"

int main(int argc, char **argv) {
    Payload2DeviceParser p;
    p.parse("ping");
    assert(p.command == PAYLOAD2DEVICE_COMMAND_PING);

    p.parse("send 1f00AA payload 112233445566ff at 2024-10-31T00:00:00+09");
    assert(p.command == PAYLOAD2DEVICE_COMMAND_SEND);
    std::string s = p.toString();
    std::cout << s << std::endl;
    // assert(s == "send 001f00aa payload 112233445566ff at 2024-10-31T00:00:00+09");
    p.parse("");

    return 0;
}
