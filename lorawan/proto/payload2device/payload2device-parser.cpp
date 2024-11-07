#include <sstream>
#include <cstring>

#include "lorawan/proto/payload2device/payload2device-parser.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-date.h"

static const char SPACE = ' ';
static const char * RESERVED_WORDS[] {
    "send",     // 0
    "ping",     // 1
    "quit",     // 2
    "payload",  // 3
    "fport",    // 4
    "fopts",    // 5
    "at",       // 6
    "proto"     // 7
};

Payload2DeviceParser::Payload2DeviceParser()
    : flagsStateClause(0), flagsStateValue(0), state(PAYLOAD2DEVICE_PARSER_STATE_START),
      lastSendOption(PAYLOAD2DEVICE_PARSER_STATE_START), command(PAYLOAD2DEVICE_COMMAND_NONE), tim(0),
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
    if (size == 0)
        size = strlen(expression);
    // clear
    lastToken = "";
    payload = "";
    fopts = "";
    tim = 0;
    flagsStateClause = 0;
    flagsStateValue = 0;
    lastSendOption = PAYLOAD2DEVICE_PARSER_STATE_START;
    state = PAYLOAD2DEVICE_PARSER_STATE_START;
    command = PAYLOAD2DEVICE_COMMAND_NONE;
    addresses.clear();

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
    if (token == RESERVED_WORDS[0]) {   // "send"
        command = PAYLOAD2DEVICE_COMMAND_SEND;
        setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_COMMAND);
    } else {
        if (token == RESERVED_WORDS[1]) {   // "ping"
            command = PAYLOAD2DEVICE_COMMAND_PING;
            setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_COMMAND);
            return command;
        } else if (token == RESERVED_WORDS[2]) { // "quit"
            command = PAYLOAD2DEVICE_COMMAND_QUIT;
            setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_COMMAND);
            return command;
        } else
            return command;             // none
    }
    state = PAYLOAD2DEVICE_PARSER_STATE_COMMAND;

    // read send options
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

        // by default address
        state = PAYLOAD2DEVICE_PARSER_STATE_ADDRESS;

        if (token == RESERVED_WORDS[4]) { // "fport"
            state = PAYLOAD2DEVICE_PARSER_STATE_FPORT;
            setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_FPORT);
            lastSendOption = PAYLOAD2DEVICE_PARSER_STATE_FPORT;
        } else {
            if (token == RESERVED_WORDS[3]) { // "payload"
                state = PAYLOAD2DEVICE_PARSER_STATE_PAYLOAD;
                setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_PAYLOAD);
                lastSendOption = PAYLOAD2DEVICE_PARSER_STATE_PAYLOAD;
            } else {
                if (token == RESERVED_WORDS[5]) { // "fopts"
                    state = PAYLOAD2DEVICE_PARSER_STATE_FOPTS;
                    setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_FOPTS);
                    lastSendOption = PAYLOAD2DEVICE_PARSER_STATE_FOPTS;
                } else {
                    if (token == RESERVED_WORDS[6]) { // "at"
                        state = PAYLOAD2DEVICE_PARSER_STATE_TIME;
                        setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_TIME);
                        lastSendOption = PAYLOAD2DEVICE_PARSER_STATE_TIME;
                    } else {
                        if (token == RESERVED_WORDS[7]) { // "proto"
                            state = PAYLOAD2DEVICE_PARSER_STATE_PROTO;
                            setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_PROTO);
                            lastSendOption = PAYLOAD2DEVICE_PARSER_STATE_PROTO;
                        }
                    }
                }
            }
        }
        if (state == PAYLOAD2DEVICE_PARSER_STATE_ADDRESS) {
            if (canInsertAddress() && isHex(token)) {
                addresses.emplace_back(token);
                setFlagSendOptionName(PAYLOAD2DEVICE_PARSER_STATE_ADDRESS);
                setFlagOptionValue(PAYLOAD2DEVICE_PARSER_STATE_ADDRESS);
                lastSendOption = PAYLOAD2DEVICE_PARSER_STATE_ADDRESS;
            } else
                lastToken = token;
            continue;
        }
        // value token
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

        switch (state) {
            case PAYLOAD2DEVICE_PARSER_STATE_FPORT:
                if (!isDec(token)) {
                    lastToken = token;
                    continue;
                }
                setFlagOptionValue(PAYLOAD2DEVICE_PARSER_STATE_FPORT);
                fport = (uint8_t) strtoul(token.c_str(), nullptr, 10);
                break;
            case PAYLOAD2DEVICE_PARSER_STATE_PAYLOAD:
                if (!isHex(token)) {
                    lastToken = token;
                    continue;
                }
                payload = hex2string(token);
                if (!payload.empty())
                    setFlagOptionValue(PAYLOAD2DEVICE_PARSER_STATE_PAYLOAD);
                break;
            case PAYLOAD2DEVICE_PARSER_STATE_FOPTS:
                if (!isHex(token)) {
                    lastToken = token;
                    continue;
                }
                fopts = hex2string(token);
                if (!fopts.empty())
                    setFlagOptionValue(PAYLOAD2DEVICE_PARSER_STATE_FOPTS);
                break;
            case PAYLOAD2DEVICE_PARSER_STATE_TIME:
                tim = parseDate(token.c_str());
                if (tim)
                    setFlagOptionValue(PAYLOAD2DEVICE_PARSER_STATE_TIME);
                else {
                    lastToken = token;
                    continue;
                }
                break;
            case PAYLOAD2DEVICE_PARSER_STATE_PROTO:
                if (!isDec(token)) {
                    lastToken = token;
                    continue;
                }
                proto = (uint8_t) strtoul(token.c_str(), nullptr, 10);
                setFlagOptionValue(PAYLOAD2DEVICE_PARSER_STATE_PROTO);
                break;
            default:
                lastToken = token;  // unknown token
                break;
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
            ss << RESERVED_WORDS[1];    // ping
            return ss.str();
        case PAYLOAD2DEVICE_COMMAND_SEND:
            ss << RESERVED_WORDS[0];    // send
            break;
        default:
            return "";
    }

    for (auto &a: addresses) {
        ss << SPACE << DEVADDR2string(a);
    }

    if (!payload.empty()) {
        ss << SPACE << RESERVED_WORDS[3] << SPACE << hexString(payload)
            << SPACE << RESERVED_WORDS[4] << SPACE << (int) fport;
    }
    if (!fopts.empty())
        ss << SPACE << RESERVED_WORDS[5] << SPACE << hexString(fopts);
    if (tim)
        ss << SPACE << RESERVED_WORDS[6] << SPACE << time2string(tim);
    if (proto > 0)
        ss << SPACE << RESERVED_WORDS[7] << SPACE << (int) proto;
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

bool Payload2DeviceParser::canInsertAddress() const
{
    return (command == PAYLOAD2DEVICE_COMMAND_SEND)
        && ((flagsStateClause & (1 << PAYLOAD2DEVICE_PARSER_STATE_FPORT)) == 0)
            && ((flagsStateClause & (1 << PAYLOAD2DEVICE_PARSER_STATE_PAYLOAD)) == 0)
               && ((flagsStateClause & (1 << PAYLOAD2DEVICE_PARSER_STATE_FOPTS)) == 0)
                  && ((flagsStateClause & (1 << PAYLOAD2DEVICE_PARSER_STATE_TIME)) == 0)
                     && ((flagsStateClause & (1 << PAYLOAD2DEVICE_PARSER_STATE_PROTO)) == 0);
}

std::string Payload2DeviceParser::complition(
    const std::string &expression
)
{
    Payload2DeviceParser parser;
    size_t len = expression.length();
    parser.parse(expression);

    if (parser.state <= PAYLOAD2DEVICE_PARSER_STATE_COMMAND) {
        if (len) {
            for (int i = 0; i < 3; i++) {
                if (strncmp(expression.c_str(), RESERVED_WORDS[i], len) == 0) {
                    return RESERVED_WORDS[i];
                }
            }
        } else {
            return RESERVED_WORDS[0];   // first one
        }
    }

    if (!parser.hasSendOptionValue(parser.lastSendOption)) {
        // add value
        return expression;
    }

    if (parser.lastToken.empty())
        return expression;

    for (int i = 3; i < 8; i++) {
        if (strncmp(parser.lastToken.c_str(), RESERVED_WORDS[i], parser.lastToken.length()) == 0) {
            return expression + RESERVED_WORDS[i][parser.lastToken.length()];
        }
    }
    return expression;
}
