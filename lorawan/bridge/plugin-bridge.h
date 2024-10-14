#ifndef TLNS_PLUGIN_BRIDGE_H
#define TLNS_PLUGIN_BRIDGE_H

#include <string>
#include "lorawan/bridge/app-bridge.h"
#include "lorawan/helper/plugin-helper.h"

/**
 * Class to load bridge from loadable modules (shared libraries, .so or .dll)
 * Loadable module must have makeBridge() function.
 * makeBridge() function return pointer to AppBridge class instance.
 * makeBridge() use extern "C" call conventions. In Windows it must declared with __declspec(dllexport)
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

/**
 * Collection of bridges loaded from shared library(.so, .dll) files
 */
class PluginBridges {
public:
    std::vector<PluginBridge> bridges;
    PluginBridges();
    int add(
        const std::string &fileName
    );
    size_t add(
        const std::vector<std::string> &fileNames
    );

    /**
     * Recursively search and load plugins from the directory
     * @param directory base directory
     * @param flags 0- as is, 1- full path, 2- relative (remove parent path)
     * @return coubt of successfully loaded plugins
     */
    size_t addDirectory(
        const std::string &directory,
        int flags
    );
    virtual ~PluginBridges();
};

#endif //TLNS_PLUGIN_BRIDGE_H
