#ifndef PLUGIN_CLIENT_H_
#define PLUGIN_CLIENT_H_	1

#include <string>

#include "direct-client.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <Windows.h>
#else
typedef void * HINSTANCE;
#endif

/**
 * Class to load specific identity and gateway services from loadable modules (shared libraries)
 * @see DirectClient
 */
class PluginClient : public DirectClient {
private:
    HINSTANCE handleSvc;
    int load(
        const std::string &fileName,
        const std::string &classIdentityName,
        const std::string &classGatewayName
    );
    void unload();
public:
    explicit PluginClient(
        const std::string &fileName,
        const std::string &classIdentityName,
        const std::string &classGatewayName
    );
    ~PluginClient() override;
};

#endif
