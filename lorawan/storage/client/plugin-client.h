#ifndef PLUGIN_CLIENT_H_
#define PLUGIN_CLIENT_H_	1

#include <string>

#include "lorawan/storage/client/direct-client.h"
#include "lorawan/helper/plugin-helper.h"

/**
 * Class to load specific identity and gateway services from loadable modules (shared libraries)
 * @see DirectClient
 */
class PluginClient : public DirectClient {
private:
    HINSTANCE handleSvc;
    int load(
        const std::string &fileName
    );
    void unload();
public:
    explicit PluginClient(
        const std::string &fileName
    );
    ~PluginClient() override;
};

#endif
