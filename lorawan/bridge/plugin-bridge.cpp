#include "lorawan/bridge/plugin-bridge.h"

#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-msg.h"
#include "lorawan/helper/file-helper.h"

#define PLUGIN_FILE_NAME_PREFIX "lib"


#ifdef ENABLE_DEBUG
#include <iostream>
#include <cstring>
#include "lorawan/lorawan-msg.h"
#endif

typedef AppBridge*(*makeBridgeFunc)();

const std::string MAKE_FUNC_PREFIX = "make";
const std::string MAKE_FUNC_IDENTITY_SUFFIX = "Bridge";

int PluginBridge::load(
    const std::string &fileName,
    const std::string &className
)
{
    if (fileName.empty())
        return ERR_CODE_LOAD_PLUGINS_FAILED;
    std::string fn(fileName);
    if (!file::fileExists(fn)) {
        if (fn.rfind(PLUGIN_FILE_NAME_SUFFIX) == std::string::npos) {
            fn += PLUGIN_FILE_NAME_SUFFIX;
        }
        if (!file::fileExists(fn)) {
            if (fn.find(PLUGIN_FILE_NAME_PREFIX) == std::string::npos) {
                fn = PLUGIN_FILE_NAME_PREFIX + fn;
            }
        }
    }
    fn = file::expandFileName(fn);

    std::string makeIdentityClass = MAKE_FUNC_PREFIX + firstCharToUpperCase(className) + MAKE_FUNC_IDENTITY_SUFFIX;
    handleSvc = dlopen(fn.c_str(), RTLD_LAZY);
    if (handleSvc) {
        auto fI = (makeBridgeFunc) dlsym(handleSvc, makeIdentityClass.c_str());
        if (fI) {
            bridge = fI();
            return CODE_OK;
        }
    }
    return ERR_CODE_LOAD_PLUGINS_FAILED;
}

void PluginBridge::unload()
{
    delete bridge;
    bridge = nullptr;

    if (handleSvc) {
        dlclose(handleSvc);
        handleSvc = nullptr;
    }
}

PluginBridge::PluginBridge(
    const std::string &fileName,
    const std::string &className
)
    : bridge(nullptr), handleSvc(nullptr)
{
    load(fileName, className);
}

PluginBridge::~PluginBridge()
{
    unload();
}
