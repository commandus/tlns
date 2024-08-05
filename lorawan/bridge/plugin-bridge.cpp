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

int PluginBridge::load(
    const std::string &fileName
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

    handleSvc = dlopen(fn.c_str(), RTLD_LAZY);
    if (handleSvc) {
        auto fI = (makeBridgeFunc) dlsym(handleSvc, "makeBridge");
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
    const std::string &fileName
)
    : bridge(nullptr), handleSvc(nullptr)
{
    load(fileName);
}

bool PluginBridge::valid()
{
    return bridge != nullptr && handleSvc != nullptr;
}

PluginBridge::~PluginBridge()
{
    unload();
}

PluginBridges::PluginBridges() = default;

int PluginBridges::add(
    const std::string &fileName
)
{
    PluginBridge pb(fileName);
    if (!pb.valid())
        return ERR_CODE_LOAD_PLUGINS_FAILED;
    bridges.push_back(std::move(pb));
    return CODE_OK;
}

void PluginBridges::add(
    const std::vector<std::string> &fileNames
)
{
    for (auto &fileName: fileNames) {
        PluginBridge pb(fileName);
        if (pb.valid())
            bridges.push_back(pb);
    }
}

PluginBridges::~PluginBridges() = default;
