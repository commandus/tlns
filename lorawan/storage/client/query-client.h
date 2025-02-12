#ifndef QUERY_CLIENT_H
#define QUERY_CLIENT_H

#include "lorawan/storage/serialization/identity-serialization.h"
#include "lorawan/storage/serialization/gateway-serialization.h"
#include "lorawan/storage/client/response-client.h"

/**
 * Abstract class
 *
 */
class QueryClient {
public:
    ResponseClient* onResponse;
    explicit QueryClient(
        ResponseClient *aOnResponse
    );

    virtual ~QueryClient();

    /**
     * Prepare to send request
     * @param value
     * @return previous message, NULL if not exists
     */
    virtual ServiceMessage* request(
        ServiceMessage* value
    ) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

#endif
