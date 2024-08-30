#include <cstring>
#include <sstream>

#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-mic.h"

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

NetworkIdentity& NetworkIdentity::operator=(
    const DEVICEID& value
)
{
    activation = value.activation;
    deviceclass = value.deviceclass;
    devEUI = value.devEUI;
    nwkSKey = value.nwkSKey;
    appSKey = value.appSKey;
    name = value.name;
    return *this;
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
	memmove(&devaddr.u, &addr.u, sizeof(DEVADDR));
	memmove(&activation, &value.activation, sizeof(activation));
	memmove(&deviceclass, &value.deviceclass, sizeof(deviceclass));
	memmove(&devEUI, &value.devEUI, sizeof(DEVEUI));
	memmove(&nwkSKey.c, &value.nwkSKey.c, sizeof(KEY128));
	memmove(&appSKey.c, &value.appSKey.c, sizeof(KEY128));
	memmove(&appEUI, &value.appEUI, sizeof(DEVEUI));
	memmove(&appKey.c, &value.appKey.c, sizeof(KEY128));
	memmove(&nwkKey.c, &value.nwkKey.c, sizeof(KEY128));
	devNonce = value.devNonce;
	memmove(&joinNonce, &value.joinNonce, sizeof(JOINNONCE));
	memmove(&name, &value.name, sizeof(DEVICENAME));
	memmove(&version, &value.version, sizeof(LORAWAN_VERSION));
}

uint32_t calculateMIC(
    const void *buf,
    size_t size,
    const NetworkIdentity &identity
)
{
    if (!buf || size < SIZE_MHDR)
        return 0;
    auto b = (uint8_t*) buf;
    switch (*((MTYPE*) buf)) {
        case MTYPE_JOIN_REQUEST:
            if (size < SIZE_MHDR + SIZE_JOIN_REQUEST_FRAME )
                return 0;
            return calculateMICJoinRequest((JOIN_REQUEST_HEADER *) (b + 1), identity.nwkSKey);
        case MTYPE_REJOIN_REQUEST:
            if (size < SIZE_MHDR + SIZE_JOIN_REQUEST_FRAME )
                return 0;
            {
                int rejoinType = 0;
                return calculateMICReJoinRequest((JOIN_REQUEST_HEADER *) (b + 1), identity.nwkSKey, rejoinType);
            }
        case MTYPE_JOIN_ACCEPT:
            if (size < SIZE_MHDR + SIZE_JOIN_ACCEPT_FRAME )
                return 0;
            return calculateMICJoinResponse(*(JOIN_ACCEPT_FRAME *) (b + 1), identity.nwkSKey);
        case MTYPE_UNCONFIRMED_DATA_UP:
        case MTYPE_CONFIRMED_DATA_UP: {
            if (size < SIZE_MHDR + SIZE_FHDR )
                return 0;
            return calculateMICFrmPayload(b, size,
                ((FHDR *) (b + 1))->fcnt, 0, identity.devaddr, identity.nwkSKey
            );
        }
        case MTYPE_UNCONFIRMED_DATA_DOWN:
        case MTYPE_CONFIRMED_DATA_DOWN: {
            if (size < SIZE_MHDR + SIZE_FHDR )
                return 0;
            return calculateMICFrmPayload(b, size,
                ((FHDR *) (b + 1))->fcnt, 1, identity.devaddr, identity.nwkSKey
            );
        }
        default:
            // case MTYPE_PROPRIETARYRADIO:
            break;
    }
    return 0;
}
