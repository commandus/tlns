#ifndef SYNC_QUERY_CLIENT_H
#define SYNC_QUERY_CLIENT_H

#include "lorawan/storage/client/query-client.h"
#include "sync-response-client.h"

/**
 * Abstract class
 *
 */
class SyncQueryClient : public QueryClient {
public:
    SyncResponseClient rc;
    SyncQueryClient();
    virtual ~SyncQueryClient();

    ServiceMessage* request(
        ServiceMessage* value
    ) override;
    void start() override;
    void stop() override;
};

#endif
