/**
 * Example shows how to use device in-memory storage
 * g++ -o example-mem -I.. example-mem.cpp -L../cmake-build-debug -llorawan
 */
#include "lorawan/storage/service/identity-service-mem.h"

int main(int argc, char *argv[])
{
    // pass master key to generate keys
    MemoryIdentityService c;
    // -- device
    // get
    DEVICEID devid;
    DEVADDR devaddr(1);
    c.get(devid, devaddr);
    if (devaddr.empty()) {
        // device not fount
    }
    // list
    std::vector<NETWORKIDENTITY> nids;
    size_t offset = 0;
    size_t size = 10;
    c.list(nids, offset, size);
    // count
    auto count = c.size();
    // put
    c.put(devaddr, devid);
    // delete
    c.rm(devaddr);
}
