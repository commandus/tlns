#ifndef PAYLOAD2DEVICE_PARSER_H
#define PAYLOAD2DEVICE_PARSER_H

#include <vector>
#include "lorawan/lorawan-types.h"

enum PAYLOAD2DEVICE_COMMAND {
    PAYLOAD2DEVICE_COMMAND_NONE = 0,
    PAYLOAD2DEVICE_COMMAND_INVALID = 1,
    PAYLOAD2DEVICE_COMMAND_PING = 2,
    PAYLOAD2DEVICE_COMMAND_SEND = 3,
    PAYLOAD2DEVICE_COMMAND_QUIT = 4
};

enum PAYLOAD2DEVICE_PARSER_STATE {
    PAYLOAD2DEVICE_PARSER_STATE_COMMAND = 0,
    PAYLOAD2DEVICE_PARSER_STATE_ADDRESS = 1,
    PAYLOAD2DEVICE_PARSER_STATE_FPORT = 2,
    PAYLOAD2DEVICE_PARSER_STATE_PAYLOAD = 3,
    PAYLOAD2DEVICE_PARSER_STATE_FOPTS = 4,
    PAYLOAD2DEVICE_PARSER_STATE_TIME = 5,
    PAYLOAD2DEVICE_PARSER_STATE_PROTO = 6
};

class Payload2DeviceParser {
public:
    PAYLOAD2DEVICE_PARSER_STATE state;
    PAYLOAD2DEVICE_COMMAND command;
    std::vector <DEVADDR> addresses;
    uint8_t fport;
    uint8_t proto;
    std::string payload;
    std::string fopts;
    time_t tim;

    Payload2DeviceParser();
    PAYLOAD2DEVICE_COMMAND parse(
        const char* expression,
        size_t size
    );
    PAYLOAD2DEVICE_COMMAND parse(
        const std::string &expression
    );
    std::string toString() const;
};

#endif
