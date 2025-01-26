#include "plugin-client.h"

#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-msg.h"
#include "lorawan/helper/file-helper.h"

#define PLUGIN_FILE_NAME_PREFIX "lib"

#if defined(_MSC_VER) || defined(__MINGW32__)
#define dlopen(fileName, opt) LoadLibraryA(fileName)
#define dlclose FreeLibrary
#define dlsym GetProcAddress
#define PLUGIN_FILE_NAME_SUFFIX ".dll"
#else
#include <dlfcn.h>
#include <algorithm>
#define PLUGIN_FILE_NAME_SUFFIX ".so"
#endif

#ifdef ENABLE_DEBUG
#include <iostream>
#include <cstring>
#include "lorawan-msg.h"
#endif

static const char * FUNC_NAME_MAKE_IDENTITY_SERVICE = "makeIdentityService";
static const char * FUNC_NAME_MAKE_GATEWAY_SERVICE = "makeGatewayService";

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
        auto fI0 = (makeIdentityServiceFunc) dlsym(handleSvc, FUNC_NAME_MAKE_IDENTITY_SERVICE);
        auto fG0 = (makeGatewayServiceFunc) dlsym(handleSvc, FUNC_NAME_MAKE_GATEWAY_SERVICE);
        if (fI0 && fG0) {
            svcIdentity = fI0();
            svcGateway = fG0();
            return CODE_OK;
        }
        // in case of static linking function name differs by last number 1..9
        for (int i = 1; i < 10; i++) {
            std::string n = std::to_string(i);
            std::string funcNameI = FUNC_NAME_MAKE_IDENTITY_SERVICE + n;
            std::string funcNameG = FUNC_NAME_MAKE_GATEWAY_SERVICE + n;
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
    const std::string &libFileName
)
	: DirectClient(), handleSvc(nullptr), fileName(libFileName)
{
    load(fileName);
}

PluginClient::PluginClient(
    const PluginClient &value
)
    : DirectClient(), handleSvc(nullptr), fileName(value.fileName)
{
    load(fileName);
}


PluginClient::~PluginClient()
{
    unload();
}
