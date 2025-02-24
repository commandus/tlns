#include <string>
#include <iostream>
#include <cassert>
#include "lorawan/proto/payload2device/payload2device-parser.h"

int main(int argc, char **argv) {
    Payload2DeviceParser p;
    const char *EXAMPLES[] {
            "send 1f00AA 1f00AA11 1f00AA22 payload 23456",
            "send 1f00AA 1f00AA11 1f00AA22 payload 23456 ",
            "send 1f00AA 1f00AA11 1f00AA22 pay",
            "send 1f00AA 1f00AA11 1f00AA22 payloa",
            "send 1f00AA 1f00AA11 1f00AA22 payload",
            "send 1f00AA 1f00AA11 1f00AA22 payload fp",
            "send 1f00AA payload 123456 ",
            "send 1f00AA pay",
            "send 1f00AA payload sdf",
            "send 1f00AA 1f00AA11 1f00AA22 payload sdf"

    };
    for (auto e : EXAMPLES) {
        p.parse(e);
        std::string c = Payload2DeviceParser::complition(e);
        std::cout << e << std::endl;
        std::cout << c << std::endl;
        std::cout << "'" << p.lastToken << "'" << std::endl;
    }

    p.parse("ping");
    assert(p.command == PAYLOAD2DEVICE_COMMAND_PING);

    p.parse("send 1f00AA payload 112233445566ff at 2024-10-31T00:00:00+09");
    assert(p.hasSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_ADDRESS));
    assert(p.hasSendOptionValue(PAYLOAD2DEVICE_PARSER_STATE_ADDRESS));
    assert(p.command == PAYLOAD2DEVICE_COMMAND_SEND);
    std::cout << p.toString() << std::endl;
    p.parse("");
    p.parse("pong  ");
    p.parse("ping\t\t");
    p.parse("send\t\t");
    std::cout << p.toString() << std::endl;
    p.parse("send at 2024-12-31T00:00:00+09\t\t");
    std::cout << p.toString() << std::endl;
    p.parse("send at 2024-12-31T00:00:00+09   proto 3  \tfport2 fport 5 \t\t");
    std::cout << p.toString() << std::endl;
    p.parse("send at 2024-12-31T00:00:00+09   proto 3  \tfport2 fport 5 \t\t payload 11\t\t\t");
    std::cout << p.toString() << std::endl;
    return 0;
}
