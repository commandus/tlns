#include <cstring>
#include <sstream>

#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-conv.h"
#include "network-identity.h"

NetworkIdentity::NetworkIdentity() = default;

NetworkIdentity::NetworkIdentity(
	const DEVADDR &a,
	const DEVICEID &value
)
    :   devaddr(a.u), activation(value.activation), deviceclass(value.deviceclass),
        devEUI(value.devEUI), nwkSKey(value.nwkSKey), appSKey(value.appSKey), name(value.name)
{
}

NetworkIdentity::NetworkIdentity
(
	const DEVICEID &value
)
    : devaddr(0), activation(value.activation), deviceclass(value.deviceclass),
          devEUI(value.devEUI), nwkSKey(value.nwkSKey), appSKey(value.appSKey), name(value.name)
{
}

std::string NetworkIdentity::toString() const
{
	std::stringstream ss;
	ss << DEVADDR2string(devaddr) 
		<< " " << activation2string(activation)
		<< " " << deviceclass2string(deviceclass)
		<< " " << DEVEUI2string(devEUI)
		<< " " << KEY2string(nwkSKey)
		<< " " << KEY2string(appSKey)
		<< " " << LORAWAN_VERSION2string(version)
        << " " << DEVEUI2string(appEUI)
        << " " << KEY2string(appKey)
        << " " << KEY2string(nwkKey)
        << " " << DEVNONCE2string(devNonce)
        << " " << JOINNONCE2string(joinNonce)
		<< " " << DEVICENAME2string(name);
	return ss.str();
}

std::string NetworkIdentity::toJsonString() const
{
    std::stringstream ss;
    ss << R"({"addr": ")" << DEVADDR2string(devaddr)
       << R"(", "activation": ")" << activation2string(activation)
       << R"(", "deviceClass": ")" << deviceclass2string(deviceclass)
       << R"(", "devEUI": ")" << DEVEUI2string(devEUI)
       << R"(", "nwkSKey": ")" << KEY2string(nwkSKey)
       << R"(", "appSKey": ")" << KEY2string(appSKey)
       << R"(", "version": ")" << LORAWAN_VERSION2string(version)
       << R"(", "appEUI": ")" << DEVEUI2string(appEUI)
       << R"(", "appKey": ")" << KEY2string(appKey)
       << R"(", "nwkKey": ")" << KEY2string(nwkKey)
       << R"(", "nonce": ")" << DEVNONCE2string(devNonce)
       << R"(", "joinNonce": ")" << JOINNONCE2string(joinNonce)
       << R"(", "name": ")" << DEVICENAME2string(name)
       << "\"}";
    return ss.str();
}

void NetworkIdentity::set(
	const DEVADDR &addr,
	const DEVICEID &value
)
{
	memmove(&devaddr.u, &addr, sizeof(DEVADDR));
	memmove(&activation, &value.activation, sizeof(activation));
	memmove(&deviceclass, &value.deviceclass, sizeof(deviceclass));
	memmove(&devEUI, &value.devEUI, sizeof(DEVEUI));
	memmove(&nwkSKey.c, &value.nwkSKey, sizeof(KEY128));
	memmove(&appSKey.c, &value.appSKey, sizeof(KEY128));
	memmove(&appEUI, &value.appEUI, sizeof(DEVEUI));
	memmove(&appKey.c, &value.appKey, sizeof(KEY128));
	memmove(&nwkKey.c, &value.nwkKey, sizeof(KEY128));
	devNonce = value.devNonce;
	memmove(&joinNonce, &value.joinNonce, sizeof(JOINNONCE));
	memmove(&name, &value.name, sizeof(DEVICENAME));
	memmove(&version, &value.version, sizeof(LORAWAN_VERSION));
}
