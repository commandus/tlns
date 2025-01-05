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
    if (isDEVADDREmpty(networkIdentity.value.devaddr)) {
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
        memmove(&retval.devAddr.c, &newNetworkIdentity.value.devaddr.c, sizeof(DEVADDR));
        memmove(&retval.joinNonce.c, &newNetworkIdentity.value.devid.id.joinNonce.c, sizeof(JOINNONCE));
        // return address in the identity too
        memmove(&networkIdentity.value.devaddr.c, &newNetworkIdentity.value.devaddr.c, sizeof(DEVADDR));
    } else {
        // re-use old address, return network identifier
        netid.get(retval.netId);
        // return address
        memmove(&retval.devAddr.c, &networkIdentity.value.devaddr.c, sizeof(DEVADDR));
        memmove(&retval.joinNonce.c, &networkIdentity.value.devid.id.joinNonce.c, sizeof(JOINNONCE));
        // return address in the identity too
        memmove(&networkIdentity.value.devaddr.c, &networkIdentity.value.devaddr.c, sizeof(DEVADDR));
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
