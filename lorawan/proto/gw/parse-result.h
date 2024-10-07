#ifndef TLNS_PARSE_RESULT_H
#define TLNS_PARSE_RESULT_H

#include "lorawan/lorawan-types.h"
#include "lorawan/proto/gw/gw.h"

class ProtoGwParser;

/**
 * ProtoGwParser::parse return result in ParseResult structure
 */
class ParseResult {
public:
    uint8_t tag;
    uint16_t token;
    GwPushData gwPushData;
    GwPullResp gwPullResp;
    ERR_CODE_TX code;           ///< code
    // pointer to the parser used for
    ProtoGwParser *parser;

    ParseResult();
};

#endif //TLNS_PARSE_RESULT_H
