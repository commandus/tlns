#include "query-client.h"

QueryClient::QueryClient(
    ResponseClient *aOnResponse
)
    : onResponse(aOnResponse)
{
}

QueryClient::~QueryClient() = default;
