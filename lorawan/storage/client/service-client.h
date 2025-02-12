#ifndef SERVICE_CLIENT_H_
#define SERVICE_CLIENT_H_	1

#include <string>

#include "direct-client.h"

/**
 * Class to load specific identity and gateway services by name (gen, mem, sqlite)
 * @see DirectClient
 */
class ServiceClient : public DirectClient {
public:
    explicit ServiceClient(
        const std::string &name
    );
    static bool hasStaticPlugin(const std::string &name);
    virtual ~ServiceClient();
};

#endif
