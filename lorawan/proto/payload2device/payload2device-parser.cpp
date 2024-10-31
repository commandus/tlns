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
    "proto"     // 6
};

Payload2DeviceParser::Payload2DeviceParser()
    : state(PAYLOAD2DEVICE_PARSER_STATE_COMMAND), command(PAYLOAD2DEVICE_COMMAND_INVALID), tim(0),
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
    if (token == RESERVED_WORDS[0]) {// "ping"
        command = PAYLOAD2DEVICE_COMMAND_PING;
        return command;
    } else
        if (token == RESERVED_WORDS[1]) // "send"
            command = PAYLOAD2DEVICE_COMMAND_SEND;
        else
            return command;

    state = PAYLOAD2DEVICE_PARSER_STATE_ADDRESS;

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
        } else {
            if (token == RESERVED_WORDS[2]) { // "payload"
                state = PAYLOAD2DEVICE_PARSER_STATE_PAYLOAD;
            } else {
                if (token == RESERVED_WORDS[3]) { // "fopts"
                    state = PAYLOAD2DEVICE_PARSER_STATE_FOPTS;
                } else {
                    if (token == RESERVED_WORDS[4]) { // "at"
                        state = PAYLOAD2DEVICE_PARSER_STATE_TIME;
                    } else {
                        if (token == RESERVED_WORDS[6]) { // "proto"
                            state = PAYLOAD2DEVICE_PARSER_STATE_PROTO;
                        } else {
                            switch (state) {
                                case PAYLOAD2DEVICE_PARSER_STATE_ADDRESS:
                                    addresses.push_back(DEVADDR(token));
                                    break;
                                case PAYLOAD2DEVICE_PARSER_STATE_FPORT:
                                    fport = (uint8_t) strtoul(token.c_str(), nullptr, 10);
                                    break;
                                case PAYLOAD2DEVICE_PARSER_STATE_PAYLOAD:
                                    payload = hex2string(token);
                                    break;
                                case PAYLOAD2DEVICE_PARSER_STATE_FOPTS:
                                    fopts = hex2string(token);
                                    break;
                                case PAYLOAD2DEVICE_PARSER_STATE_TIME:
                                    tim = parseDate(token.c_str());
                                    break;
                                case PAYLOAD2DEVICE_PARSER_STATE_PROTO:
                                    proto = (uint8_t) strtoul(token.c_str(), nullptr, 10);
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
