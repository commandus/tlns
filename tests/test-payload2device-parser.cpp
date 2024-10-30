#include <string>
#include <iostream>
#include <cassert>
#include "lorawan/proto/payload2device/payload2device-parser.h"

int main(int argc, char **argv) {
    Payload2DeviceParser p;
    std::string expr("ping");
    p.parse(expr);
    assert(p.command == PAYLOAD2DEVICE_COMMAND_PING);

    p.parse("send 1f00AA payload 112233445566ff at 2024-10-31T00:00:00+09");
    assert(p.command == PAYLOAD2DEVICE_COMMAND_SEND);

    std::cout << p.toString() << std::endl;

    return 0;
}
