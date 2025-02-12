/**
 * g++ -o example-direct -I.. example-direct.cpp -L../cmake-build-debug -llorawan
 */
#include "lorawan/storage/client/service-client.h"

int main(int argc, char *argv[])
{
    std::string masterkey = "masterkey";
    ServiceClient c("gen");
    // 0- pass master key to generate keys
    c.svcIdentity->setOption(0, &masterkey);
    
    // -- device
    // get
    DEVICEID devid;
    DEVADDR devaddr(1);
    c.svcIdentity->get(devid, devaddr);
    if (devaddr.empty()) {
        // device not fount
    }
    // list
    std::vector<NETWORKIDENTITY> nids;
    size_t offset = 0;
    size_t size = 10;
    c.svcIdentity->list(nids, offset, size);
    // count
    auto count = c.svcIdentity->size();
    // put
    c.svcIdentity->put(devaddr, devid);
    // delete
    c.svcIdentity->rm(devaddr);
    
    // -- gateway
    // get
    GatewayIdentity gid;
    gid.gatewayId = 0x11223344;
    c.svcGateway->get(gid, gid);
    if (!gid.gatewayId) {
        // gateway not fount
    }
    // list
    std::vector <GatewayIdentity> gids;
    c.svcGateway->list(gids, offset, size);
    auto sz = c.svcGateway->size();
    // assign
    c.svcGateway->put(gid);
    // delete
    c.svcGateway->rm(gid);
}
