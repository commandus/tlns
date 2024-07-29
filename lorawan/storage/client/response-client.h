#ifndef RESPONSE_CLIENT_H
#define RESPONSE_CLIENT_H

#include <cinttypes>

class QueryClient;
class IdentityGetResponse;
class IdentityOperationResponse;
class IdentityListResponse;
class GatewayGetResponse;
class GatewayOperationResponse;
class GatewayListResponse;

/**
 * Asynchronous response abstract class
 */
class ResponseClient {
public:
    virtual void onIdentityGet(
        QueryClient* client,
        const IdentityGetResponse *response
    ) = 0;
    virtual void onIdentityOperation(
        QueryClient* client,
        const IdentityOperationResponse *response
    ) = 0;
    virtual void onIdentityList(
        QueryClient* client,
        const IdentityListResponse *response
    ) = 0;

    virtual void onGatewayGet(
        QueryClient* client,
        const GatewayGetResponse *response
    ) = 0;
    virtual void onGatewayOperation(
        QueryClient* client,
        const GatewayOperationResponse *response
    ) = 0;
    virtual void onGatewayList(
        QueryClient* client,
        const GatewayListResponse *response
    ) = 0;
    virtual void onError(
        QueryClient* client,
        int32_t code,  // 0- success, != 0- failure (error code)
        int errorCode
    ) = 0;
    // TCP connection lost
    virtual void onDisconnected(
        QueryClient* client
    ) = 0;
};

#endif
