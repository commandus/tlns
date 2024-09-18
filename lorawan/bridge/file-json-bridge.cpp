#include <iostream>
#include <fstream>
#include "lorawan/bridge/file-json-bridge.h"
#include "lorawan/lorawan-string.h"

void FileJsonBridge::onPayload(
    const void *dispatcher,
    const MessageQueueItem *messageItem,
    bool decoded
)
{
    if (strm && messageItem)
        *strm << messageItem->toJsonString() << std::endl;
}

void FileJsonBridge::init(
    const std::string& option,
    const std::string& option2,
    const void *option3
)
{
    fileName = option;
    strm = new std::fstream(fileName, std::ios_base::out);
    if (!strm || !strm->is_open()) {
        delete strm;
        strm = nullptr;
    }
}

void FileJsonBridge::done()
{
    if (strm) {
        strm->flush();
        delete strm;
        strm = nullptr;
    }
}

EXPORT_SHARED_C_FUNC AppBridge* makeBridge2()
{
    return new FileJsonBridge;
}
