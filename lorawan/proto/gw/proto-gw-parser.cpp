#include "lorawan/proto/gw/proto-gw-parser.h"

ParseResult::ParseResult()
    : tag(0), token(0), code(ERR_CODE_TX::JIT_TX_OK)
{
}


ProtoGwParser::ProtoGwParser(
    MessageTaskDispatcher *aDispatcher
)
    : dispatcher(aDispatcher)
{

}

ProtoGwParser::~ProtoGwParser()
{

}
