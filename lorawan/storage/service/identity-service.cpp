#include <cstring>
#include "lorawan/storage/service/identity-service.h"
#include "lorawan/lorawan-conv.h"

IdentityService::IdentityService()
    : responseClient(nullptr)
{

}

IdentityService::IdentityService(
    const IdentityService &value
)
    : netid(value.netid), responseClient(value.responseClient)
{

}

IdentityService::IdentityService(
    ResponseClient *aResponseClient
)
    : responseClient(aResponseClient)
{

}

IdentityService::~IdentityService() = default;

int IdentityService::joinAccept(
    JOIN_ACCEPT_FRAME_HEADER &retval,
    NETWORKIDENTITY &networkIdentity
) {
    if (isDEVADDREmpty(networkIdentity.devaddr)) {
        // No assigned address, generate a new one
        // return network identifier
        netid.get(retval.netId);
        NETWORKIDENTITY newNetworkIdentity(networkIdentity);
        int r = this->next(newNetworkIdentity);
        if (r)
            return r;
        // return network identifier
        netid.get(retval.netId);
        // return address
        memmove(&retval.devAddr.c, &newNetworkIdentity.devaddr.c, sizeof(DEVADDR));
        memmove(&retval.joinNonce.c, &newNetworkIdentity.devid.joinNonce.c, sizeof(JOINNONCE));
        // return address in the identity too
        memmove(&networkIdentity.devaddr.c, &newNetworkIdentity.devaddr.c, sizeof(DEVADDR));
    } else {
        // re-use old address, return network identifier
        netid.get(retval.netId);
        // return address
        memmove(&retval.devAddr.c, &networkIdentity.devaddr.c, sizeof(DEVADDR));
        memmove(&retval.joinNonce.c, &networkIdentity.devid.joinNonce.c, sizeof(JOINNONCE));
        // return address in the identity too
        memmove(&networkIdentity.devaddr.c, &networkIdentity.devaddr.c, sizeof(DEVADDR));
    }
    return 0;
}

NETID *IdentityService::getNetworkId() {
    return &netid;
}

void IdentityService::setNetworkId(
    const NETID &value
) {
    netid.set(value);
}
