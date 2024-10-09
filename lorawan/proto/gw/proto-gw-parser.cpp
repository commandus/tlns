#include <sstream>
#include "lorawan/proto/gw/proto-gw-parser.h"

ProtoGwParser::ProtoGwParser(
    MessageTaskDispatcher *aDispatcher
)
    : dispatcher(aDispatcher)
{

}

ProtoGwParser::~ProtoGwParser()
{

}

const std::string ProtoGwParser::toJsonString() const {
    std::stringstream ss;
    ss << "{\"tag\": " << tag() << ", \"name\": \"" << name() << "\"}";
    return ss.str();
}
