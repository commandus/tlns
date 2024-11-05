#include <sstream>

#include "lorawan/proto/payload2device/payload2device-parser.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-date.h"

static const char SPACE = ' ';
static const std::string RESERVED_WORDS[] {
    "ping",     // 0
    "send",     // 1
    "payload",  // 2
    "fopts",    // 3
    "at",       // 4
    "fport",    // 5
    "proto",    // 6
    "quit"      // 7
};

Payload2DeviceParser::Payload2DeviceParser()
    : flagsStateClause(0), flagsStateValue(0), state(PAYLOAD2DEVICE_PARSER_STATE_COMMAND),
      lastSendOption(PAYLOAD2DEVICE_PARSER_STATE_COMMAND), command(PAYLOAD2DEVICE_COMMAND_INVALID), tim(0),
      fport(1), proto(0)
{

}

PAYLOAD2DEVICE_COMMAND Payload2DeviceParser::parse(
    const std::string &expression
)
{
    return parse(expression.c_str(), expression.size());
}

/**
 * send <address> [payload <hex-string>] [fopts <hex-string>] [at <date-time>]
 * @param expression
 * @param size
 * @return
 */
PAYLOAD2DEVICE_COMMAND Payload2DeviceParser::parse(
    const char* expression,
    size_t size
)
{
    // clear
    payload = "";
    fopts = "";
    tim = 0;
    lastSendOption = PAYLOAD2DEVICE_PARSER_STATE_COMMAND;
    flagsStateClause = 0;
    flagsStateValue = 0;
    addresses.clear();

    command = PAYLOAD2DEVICE_COMMAND_INVALID;

    size_t start = 0;
    size_t eolp = size;
    size_t finish = eolp;

    // skip spaces if exists
    for (auto p = start; p < eolp; p++) {
        if (!std::isspace(expression[p])) {
            start = p;
            break;
        }
    }

    // read command first
    for (auto p = start; p < eolp; p++) {
        if (!isalpha(expression[p])) {
            finish = p;
            break;
        }
    }

    std::string token(expression + start, finish - start);
    if (token == RESERVED_WORDS[0]) {   // "ping"
        command = PAYLOAD2DEVICE_COMMAND_PING;
        setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_COMMAND);
        return command;
    } else
        if (token == RESERVED_WORDS[7]) {   // "quit"
            command = PAYLOAD2DEVICE_COMMAND_QUIT;
            setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_COMMAND);
            return command;
        } else
            if (token == RESERVED_WORDS[1]) { // "send"
                command = PAYLOAD2DEVICE_COMMAND_SEND;
                setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_COMMAND);
            } else
                return command;             // none

    state = PAYLOAD2DEVICE_PARSER_STATE_COMMAND;

    while (finish < size ) {
        start = finish;
        // skip spaces if exists
        for (auto p = start; p < eolp; p++) {
            if (!std::isspace(expression[p])) {
                start = p;
                break;
            }
        }
        finish = eolp;
        // try read token
        for (auto p = start; p < eolp; p++) {
            if (isspace(expression[p])) {
                finish = p;
                break;
            }
        }
        token = std::string(expression + start, finish - start);
        if (token.empty())
            return command;
        if (token == RESERVED_WORDS[5]) { // "fport"
            state = PAYLOAD2DEVICE_PARSER_STATE_FPORT;
            setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_FPORT);
            lastSendOption = PAYLOAD2DEVICE_PARSER_STATE_FPORT;
        } else {
            if (token == RESERVED_WORDS[2]) { // "payload"
                state = PAYLOAD2DEVICE_PARSER_STATE_PAYLOAD;
                setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_PAYLOAD);
                lastSendOption = PAYLOAD2DEVICE_PARSER_STATE_PAYLOAD;
            } else {
                if (token == RESERVED_WORDS[3]) { // "fopts"
                    state = PAYLOAD2DEVICE_PARSER_STATE_FOPTS;
                    setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_FOPTS);
                    lastSendOption = PAYLOAD2DEVICE_PARSER_STATE_FOPTS;
                } else {
                    if (token == RESERVED_WORDS[4]) { // "at"
                        state = PAYLOAD2DEVICE_PARSER_STATE_TIME;
                        setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_TIME);
                        lastSendOption = PAYLOAD2DEVICE_PARSER_STATE_TIME;
                    } else {
                        if (token == RESERVED_WORDS[6]) { // "proto"
                            state = PAYLOAD2DEVICE_PARSER_STATE_PROTO;
                            setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_PROTO);
                            lastSendOption = PAYLOAD2DEVICE_PARSER_STATE_PROTO;
                        } else {
                            switch (state) {
                                case PAYLOAD2DEVICE_PARSER_STATE_ADDRESS:
                                    setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_ADDRESS);
                                    setFlagOptionValue(PAYLOAD2DEVICE_PARSER_STATE_ADDRESS);
                                    lastSendOption = PAYLOAD2DEVICE_PARSER_STATE_ADDRESS;
                                    addresses.emplace_back(token);
                                    break;
                                case PAYLOAD2DEVICE_PARSER_STATE_FPORT:
                                    setFlagOptionValue(PAYLOAD2DEVICE_PARSER_STATE_FPORT);
                                    fport = (uint8_t) strtoul(token.c_str(), nullptr, 10);
                                    break;
                                case PAYLOAD2DEVICE_PARSER_STATE_PAYLOAD:
                                    payload = hex2string(token);
                                    if (!payload.empty())
                                        setFlagOptionValue(PAYLOAD2DEVICE_PARSER_STATE_PAYLOAD);
                                    break;
                                case PAYLOAD2DEVICE_PARSER_STATE_FOPTS:
                                    fopts = hex2string(token);
                                    if (!fopts.empty())
                                        setFlagOptionValue(PAYLOAD2DEVICE_PARSER_STATE_FOPTS);
                                    break;
                                case PAYLOAD2DEVICE_PARSER_STATE_TIME:
                                    tim = parseDate(token.c_str());
                                    if (tim)
                                        setFlagOptionValue(PAYLOAD2DEVICE_PARSER_STATE_TIME);
                                    break;
                                case PAYLOAD2DEVICE_PARSER_STATE_PROTO:
                                    proto = (uint8_t) strtoul(token.c_str(), nullptr, 10);
                                    setFlagOptionValue(PAYLOAD2DEVICE_PARSER_STATE_PROTO);
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
            }
        }
    }
    // fix FPort if no payload set FPort to MAC payload (0)
    if (payload.empty())
        fport = 0;
    return command;
}

std::string Payload2DeviceParser::toString() const
{
    std::stringstream ss;
    switch (command) {
        case PAYLOAD2DEVICE_COMMAND_PING:
            ss << RESERVED_WORDS[0];
            return ss.str();
        case PAYLOAD2DEVICE_COMMAND_SEND:
            ss << RESERVED_WORDS[1];
            break;
        default:
            return "";
    }

    for (auto &a: addresses) {
        ss  << SPACE << DEVADDR2string(a);
    }

    if (!payload.empty()) {
        ss << SPACE << RESERVED_WORDS[5] << SPACE << (int) fport
            << SPACE << RESERVED_WORDS[2] << SPACE << hexString(payload);
    }
    if (!fopts.empty())
        ss << SPACE << RESERVED_WORDS[3] << SPACE << hexString(fopts);
    if (tim)
        ss << SPACE << RESERVED_WORDS[4] << SPACE << time2string(tim);
    if (proto > 0)
        ss << SPACE << RESERVED_WORDS[6] << SPACE << (int) proto;
    return ss.str();
}

bool Payload2DeviceParser::hasSendOptionName(
    PAYLOAD2DEVICE_PARSER_STATE state
)
{
    return ((1 << state) & flagsStateClause);
}

bool Payload2DeviceParser::hasSendOptionValue(
    PAYLOAD2DEVICE_PARSER_STATE state
)
{
    return ((1 << state) & flagsStateValue);
}

void Payload2DeviceParser::setFlagSendOptionName(
    PAYLOAD2DEVICE_PARSER_STATE state
)
{
    flagsStateClause |= (1 << state);
}

void Payload2DeviceParser::setFlagOptionValue(
    PAYLOAD2DEVICE_PARSER_STATE state
)
{
    flagsStateValue |= (1 << state);
}
