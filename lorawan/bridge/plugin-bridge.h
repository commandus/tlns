#ifndef TLNS_PLUGIN_BRIDGE_H
#define TLNS_PLUGIN_BRIDGE_H

#include <string>
#include "lorawan/bridge/app-bridge.h"
#include "lorawan/helper/plugin-helper.h"

/**
 * Class to load bridge from loadable modules (shared libraries)
 * @see AppBridge
 */
class PluginBridge {
private:
    std::string fileName;
    HINSTANCE handleSvc;
    int load(
        const std::string &fileName
    );
    void unload();
public:
    AppBridge *bridge;
    explicit PluginBridge(
        const std::string &fileName
    );
    PluginBridge(const PluginBridge &value);
    bool valid();
    virtual ~PluginBridge();
};

class PluginBridges {
public:
    std::vector<PluginBridge> bridges;
    PluginBridges();
    int add(
        const std::string &fileName
    );
    void add(
        const std::vector<std::string> &fileNames
    );
    virtual ~PluginBridges();
};

#endif //TLNS_PLUGIN_BRIDGE_H
