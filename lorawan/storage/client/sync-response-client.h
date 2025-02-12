#ifndef SYNC_RESPONSE_CLIENT_H
#define SYNC_RESPONSE_CLIENT_H

#include "response-client.h"

/**
 * synchronous wrapper for async
 */
class SyncResponseClient : public ResponseClient {
public:
    void onIdentityGet(
        QueryClient* client,
        const IdentityGetResponse *response
    ) override;
    void onIdentityOperation(
        QueryClient* client,
        const IdentityOperationResponse *response
    ) override;
    void onIdentityList(
        QueryClient* client,
        const IdentityListResponse *response
    ) override;

    void onGatewayGet(
        QueryClient* client,
        const GatewayGetResponse *response
    ) override;
    void onGatewayOperation(
        QueryClient* client,
        const GatewayOperationResponse *response
    ) override;
    void onGatewayList(
        QueryClient* client,
        const GatewayListResponse *response
    ) override;
    void onError(
        QueryClient* client,
        int32_t code,  // 0- success, != 0- failure (error code)
        int errorCode
    ) override;
    // TCP connection lost
    void onDisconnected(
        QueryClient* client
    ) override;
};

#endif
