#ifndef PLUGIN_CLIENT_H_
#define PLUGIN_CLIENT_H_	1

#include <string>
#include "lorawan/storage/serialization/gateway-serialization.h"
#include "lorawan/storage/client/query-client.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <Windows.h>
#define PLUGIN_FILE_NAME_SUFFIX ".dll"
#else
#define PLUGIN_FILE_NAME_SUFFIX ".so"
typedef void * HINSTANCE;
#endif

class PluginQueryClient : public QueryClient {
private:
    HINSTANCE handleSvc;
    IdentityService *svcIdentity;
    GatewayService *svcGateway;
    int status;
    ServiceMessage* query;
    int32_t code;
    uint64_t accessCode;
    int load(
        const std::string &fileName,
        const std::string &identityClassName,
        const std::string &gatewayClassName
    );
    void unload();
public:
    explicit PluginQueryClient(
            const std::string &fileName,
            const std::string &identityClassName,
            const std::string &gatewayClassName,
            ResponseClient *onResponse,
            int32_t code,
            uint64_t accessCode
    );
    ~PluginQueryClient() override;

    /**
     * Prepare to send request
     * @param value
     * @return previous message, NULL if not exists
     */
    ServiceMessage* request(
        ServiceMessage* value
    ) override;
    void start() override;
    void stop() override;
};

#endif
