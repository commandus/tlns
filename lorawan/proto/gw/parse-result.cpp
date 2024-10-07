#include "lorawan/proto/gw/parse-result.h"

ParseResult::ParseResult()
    : tag(0), token(0), code(ERR_CODE_TX::JIT_TX_OK), parser(nullptr)
{
}
