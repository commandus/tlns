#include <iostream>
#include <fstream>
#include "lorawan/bridge/file-json-bridge.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-error.h"

static const char *APP_BRIDGE_NAME = "stdout-app-bridge";

FileJsonBridge::FileJsonBridge() = default;

void FileJsonBridge::onPayload(
    const void *dispatcher,
    const MessageQueueItem *messageItem,
    bool decoded,
    bool micMatched
)
{
    if (strm && messageItem)
        *strm << messageItem->toJsonString() << std::endl;
}

int FileJsonBridge::init(
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
    return CODE_OK;
}

void FileJsonBridge::done()
{
    if (strm) {
        strm->flush();
        delete strm;
        strm = nullptr;
    }
}

void FileJsonBridge::onSend(
    const void *dispatcher,
    const MessageQueueItem *item,
    int code
)
{
    if (item)
        std::cerr << "Sent " << item->toString() << " with code " << code << std::endl;
}

const char *FileJsonBridge::name()  
{
    return APP_BRIDGE_NAME;
}

EXPORT_SHARED_C_FUNC AppBridge* makeBridge2()
{
    return new FileJsonBridge;
}
