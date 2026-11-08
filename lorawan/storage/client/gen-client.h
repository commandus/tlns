#ifndef GEN_CLIENT_H_
#define GEN_CLIENT_H_	1

#include <string>

#include "direct-client.h"

/**
 * Class generates identity and gateway
 * @see DirectClient
 */
class GenClient : public DirectClient {
public:
    explicit GenClient(
        const std::string &masterKey
    );
    explicit GenClient(
        const std::string &masterKey,
        const GatewayIdentity &gateway
    );
    virtual ~GenClient();
};

#endif
