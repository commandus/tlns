/**
 * Example shows how to use device key generator
 * g++ -o example-gen -I.. example-gen.cpp -L../cmake-build-debug -llorawan
 */
#include <iostream>
#include "lorawan/lorawan-string.h"
#include "lorawan/storage/service/identity-service-gen.h"

int main(int argc, char *argv[])
{
    // pass master key to generate keys
    GenIdentityService c("masterkey");
    // -- device
    // get
    DEVICEID devid;
    DEVADDR devaddr(1);
    c.get(devid, devaddr);
    if (devaddr.empty()) {
        // device not fount
    }
    // count
    auto count = c.size();
    std::cout << "Total " << count << " addresses" << std::endl;

    // list
    std::vector<NETWORKIDENTITY> nids;
    size_t offset = 0;
    size_t size = 10;
    c.list(nids, offset, size);
    std::cout << "Address\t\tDevice EUI\t\t\tApp EUI\t\t\t\tApp Key\t\t\t\t\t\t\t\tNetwork Key\t\t\t\t\t\t\tnwkSKey\t\t\t\t\t\t\t\tappSKey\n";
    for (auto n: nids) {
        std::cout << DEVADDR2string(n.value.devaddr)
            << '\t' << DEVEUI2string(n.value.devid.id.devEUI)
            << '\t' << DEVEUI2string(n.value.devid.id.appEUI)
            << '\t' << KEY2string(n.value.devid.id.nwkKey)
            << '\t' << KEY2string(n.value.devid.id.appKey)
            << '\t' << KEY2string(n.value.devid.id.nwkSKey)
            << '\t' << KEY2string(n.value.devid.id.appSKey)
            << '\n';
    }

    // put
    c.put(devaddr, devid);
    // delete
    c.rm(devaddr);
}
