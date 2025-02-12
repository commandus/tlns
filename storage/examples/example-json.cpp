/**
 * Example shows how to use device in-memory storage stored in JSON file
 * g++ -o example-json -I.. example-json.cpp -L../cmake-build-debug -llorawan
 */
#include <iostream>
#include "lorawan/storage/service/identity-service-json.h"
#include "lorawan/storage/service/gateway-service-json.h"

static void printIdentity(
    const std::string &fileName
) {
    JsonIdentityService c;
    c.init(fileName, nullptr);
    std::cout << "Device count: " << c.size() << std::endl;
    std::vector<NETWORKIDENTITY> l;
    c.list(l, 0, c.size());
    for (auto it: l) {
        std::cout << it.toString() << std::endl;
    }
    // rewrite file
    c.flush();
}

static void printGateway(
    const std::string &fileName
) {
    JsonGatewayService c;
    c.init(fileName, nullptr);
    std::cout << "Gateway count: " << c.size() << std::endl;
    std::vector<GatewayIdentity> l;
    c.list(l, 0, c.size());
    for (auto it: l) {
        std::cout << it.toString() << std::endl;
    }
    // rewrite file
    c.flush();
}

int main(int argc, char *argv[])
{
    printIdentity("identity.json");
    printGateway("gateway.json");
}
