#include "plugin-client.h"

#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-conv.h"
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

typedef IdentityService*(*makeIdentityServiceFunc)();
typedef GatewayService*(*makeGatewayServiceFunc)();

const std::string MAKE_FUNC_PREFIX = "make";
const std::string MAKE_FUNC_IDENTITY_SUFFIX = "IdentityService";
const std::string MAKE_FUNC_GATEWAY_SUFFIX = "GatewayService";

int PluginClient::load(
    const std::string &fileName,
    const std::string &classIdentityName,
    const std::string &classGatewayName
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

    std::string makeIdentityClass = MAKE_FUNC_PREFIX + firstCharToUpperCase(classIdentityName) + MAKE_FUNC_IDENTITY_SUFFIX;
    std::string makeGatewayClass = MAKE_FUNC_PREFIX + firstCharToUpperCase(classGatewayName) + MAKE_FUNC_GATEWAY_SUFFIX;
    handleSvc = dlopen(fn.c_str(), RTLD_LAZY);
    if (handleSvc) {
        auto fI = (makeIdentityServiceFunc) dlsym(handleSvc, makeIdentityClass.c_str());
        auto fG = (makeGatewayServiceFunc) dlsym(handleSvc, makeGatewayClass.c_str());
        if (fI && fG) {
            svcIdentity = fI();
            svcGateway = fG();
            return CODE_OK;
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
    const std::string &fileName,
    const std::string &classIdentityName,
    const std::string &classGatewayName
)
	: DirectClient(), handleSvc(nullptr)
{
    load(fileName, classIdentityName, classGatewayName);
}

PluginClient::~PluginClient()
{
    unload();
}
