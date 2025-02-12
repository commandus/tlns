#include "sync-response-client.h"

void SyncResponseClient::onIdentityGet(
    QueryClient* client,
    const IdentityGetResponse *response
)
{
}

void SyncResponseClient::onIdentityOperation(
    QueryClient* client,
    const IdentityOperationResponse *response
)
{
}

void SyncResponseClient::onIdentityList(
    QueryClient* client,
    const IdentityListResponse *response
)
{
}

void SyncResponseClient::onGatewayGet(
    QueryClient* client,
    const GatewayGetResponse *response
)
{
}

void SyncResponseClient::onGatewayOperation(
    QueryClient* client,
    const GatewayOperationResponse *response
)
{
}

void SyncResponseClient::onGatewayList(
    QueryClient* client,
    const GatewayListResponse *response
)
{
}

void SyncResponseClient::onError(
    QueryClient* client,
    int32_t code,  // 0- success, != 0- failure (error code)
    int errorCode
)
{
}

// TCP connection lost
void SyncResponseClient::onDisconnected(
    QueryClient* client
)
{
}
