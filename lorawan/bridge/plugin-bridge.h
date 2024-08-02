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
    HINSTANCE handleSvc;
    AppBridge *bridge;
    int load(
        const std::string &fileName,
        const std::string &className
    );
    void unload();
public:
    explicit PluginBridge(
        const std::string &fileName,
        const std::string &className
    );
    bool valid();
    virtual ~PluginBridge();
};

class PluginBridges {
private:
    std::vector<PluginBridge> bridges;
public:
    PluginBridges();
    int add(
        const std::string &fileName,
        const std::string &className
    );
    virtual ~PluginBridges();
};

#endif //TLNS_PLUGIN_BRIDGE_H
