/**
 * Example shows how to use gateway storage
 * g++ -o example-gw-mem -I.. example-gw-mem.cpp -L../cmake-build-debug -llorawan
 */
#include "lorawan/storage/service/gateway-service-mem.h"

int main(int argc, char *argv[])
{
    MemoryGatewayService c;
    // get
    GatewayIdentity gid;
    gid.gatewayId = 0x11223344;
    c.get(gid, gid);
    if (!gid.gatewayId) {
        // gateway not fount
    }
    // list
    std::vector <GatewayIdentity> gids;
    c.list(gids, 0, 10);
    auto sz = c.size();
    // assign
    c.put(gid);
    // delete
    c.rm(gid);
}
