#include "plugin-client.h"

#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/lorawan-msg.h"
#include "lorawan/helper/file-helper.h"

#define PLUGIN_FILE_NAME_PREFIX "lib"


#ifdef ENABLE_DEBUG
#include <iostream>
#include <cstring>
#include "lorawan/lorawan-msg.h"
#endif

typedef IdentityService*(*makeIdentityServiceFunc)();
typedef GatewayService*(*makeGatewayServiceFunc)();

int PluginClient::load(
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
        auto fI = (makeIdentityServiceFunc) dlsym(handleSvc, "makeIdentityClient");
        auto fG = (makeGatewayServiceFunc) dlsym(handleSvc, "makeGatewayClient");
        if (fI && fG) {
            svcIdentity = fI();
            svcGateway = fG();
            return CODE_OK;
        }
        // in case of static linking function name differs by last number 1..9
        for (int i = 1; i < 10; i++) {
            std::string n = std::to_string(i);
            std::string funcNameI = "makeIdentityClient" + n;
            std::string funcNameG = "makeGatewayClient" + n;
            auto fI = (makeIdentityServiceFunc) dlsym(handleSvc, funcNameI.c_str());
            auto fG = (makeGatewayServiceFunc) dlsym(handleSvc, funcNameG.c_str());
            if (fI && fG) {
                svcIdentity = fI();
                svcGateway = fG();
                return CODE_OK;
            }
        }
    }
    return ERR_CODE_LOAD_PLUGINS_FAILED;
}

void PluginClient::unload()
{
    delete svcIdentity;
    svcIdentity = nullptr;
    delete svcGateway;
    svcGateway = nullptr;

    if (handleSvc) {
        dlclose(handleSvc);
        handleSvc = nullptr;
    }
}

PluginClient::PluginClient(
    const std::string &fileName
)
	: DirectClient(), handleSvc(nullptr)
{
    load(fileName);
}

PluginClient::~PluginClient()
{
    unload();
}
