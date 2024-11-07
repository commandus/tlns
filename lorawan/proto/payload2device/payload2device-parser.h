#ifndef PAYLOAD2DEVICE_PARSER_H
#define PAYLOAD2DEVICE_PARSER_H

#include <vector>
#include "lorawan/lorawan-types.h"

enum PAYLOAD2DEVICE_COMMAND {
    PAYLOAD2DEVICE_COMMAND_NONE = 0,
    PAYLOAD2DEVICE_COMMAND_PING = 1,
    PAYLOAD2DEVICE_COMMAND_SEND = 2,
    PAYLOAD2DEVICE_COMMAND_QUIT = 3
};

enum PAYLOAD2DEVICE_PARSER_STATE {
    PAYLOAD2DEVICE_PARSER_STATE_START = 0,
    PAYLOAD2DEVICE_PARSER_STATE_COMMAND = 1,
    PAYLOAD2DEVICE_PARSER_STATE_ADDRESS = 2,
    PAYLOAD2DEVICE_PARSER_STATE_FPORT = 3,
    PAYLOAD2DEVICE_PARSER_STATE_PAYLOAD = 4,
    PAYLOAD2DEVICE_PARSER_STATE_FOPTS = 5,
    PAYLOAD2DEVICE_PARSER_STATE_TIME = 6,
    PAYLOAD2DEVICE_PARSER_STATE_PROTO = 7
};

class Payload2DeviceParser {
private:
    uint8_t flagsStateClause;
    uint8_t flagsStateValue;
    void setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE state);
    void setFlagOptionValue(PAYLOAD2DEVICE_PARSER_STATE state);
public:
    PAYLOAD2DEVICE_PARSER_STATE state;
    PAYLOAD2DEVICE_PARSER_STATE lastSendOption;
    std::string lastToken;
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
        size_t size = 0
    );
    PAYLOAD2DEVICE_COMMAND parse(
        const std::string &expression
    );
    std::string toString() const;

    bool hasSendOptionName(PAYLOAD2DEVICE_PARSER_STATE state);
    bool hasSendOptionValue(PAYLOAD2DEVICE_PARSER_STATE state);
    bool canInsertAddress() const;

    static std::string complition(const std::string &expression);
};

#endif
