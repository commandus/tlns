#include <cstring>
#include <sstream>
#include <lorawan/lorawan-msg.h>

#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-mic.h"

NetworkIdentity::NetworkIdentity() = default;

NetworkIdentity::NetworkIdentity(
	const DEVADDR &a,
	const DEVICEID &value
)
    :   devaddr(a.u), activation(value.id.activation), deviceclass(value.id.deviceclass),
        devEUI(value.id.devEUI), nwkSKey(value.id.nwkSKey), appSKey(value.id.appSKey),
        version(value.id.version),
        appEUI(value.id.appEUI), appKey(value.id.appKey), nwkKey(value.id.nwkKey),
        devNonce(value.id.devNonce), token(value.id.token), region(value.id.region), subRegion(value.id.subRegion),
        name(value.id.name)
{
}

NetworkIdentity::NetworkIdentity(
	const DEVICEID &value
)
    : devaddr(0), activation(value.id.activation), deviceclass(value.id.deviceclass),
    devEUI(value.id.devEUI), nwkSKey(value.id.nwkSKey), appSKey(value.id.appSKey),
    version(value.id.version),
    appEUI(value.id.appEUI), appKey(value.id.appKey), nwkKey(value.id.nwkKey),
    devNonce(value.id.devNonce), token(value.id.token), region(value.id.region), subRegion(value.id.subRegion),
    name(value.id.name)
{
}

NetworkIdentity& NetworkIdentity::operator=(
    const DEVICEID& value
)
{
    activation = value.id.activation;
    deviceclass = value.id.deviceclass;
    devEUI = value.id.devEUI;
    nwkSKey = value.id.nwkSKey;
    appSKey = value.id.appSKey;
    name = value.id.name;
    version = value.id.version;
    appEUI = value.id.appEUI;
    appKey = value.id.appKey;
    nwkKey = value.id.nwkKey;
    devNonce = value.id.devNonce;
    token = value.id.token;
    region = value.id.region;
    subRegion = value.id.subRegion;
    return *this;
}

std::string NetworkIdentity::toString() const
{
	std::stringstream ss;
	ss << DEVADDR2string(devaddr) 
		<< MSG_SPACE << activation2string(activation)
		<< MSG_SPACE << deviceclass2string(deviceclass)
		<< MSG_SPACE << DEVEUI2string(devEUI)
		<< MSG_SPACE << KEY2string(nwkSKey)
		<< MSG_SPACE << KEY2string(appSKey)
		<< MSG_SPACE << LORAWAN_VERSION2string(version)
        << MSG_SPACE << DEVEUI2string(appEUI)
        << MSG_SPACE << KEY2string(appKey)
        << MSG_SPACE << KEY2string(nwkKey)
        << MSG_SPACE << DEVNONCE2string(devNonce)
        << MSG_SPACE << JOINNONCE2string(joinNonce)
        << MSG_SPACE << token2string(token)
        << MSG_SPACE << region2string(region)
        << MSG_SPACE << subRegion2string(subRegion)
		<< MSG_SPACE << DEVICENAME2string(name);
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
       << R"(", "token": )" << token2string(token)
       << R"(, "region": )" << region2string(region)
       << R"(, "subRegion": )" << subRegion2string(subRegion)
       << R"(, "name": ")" << DEVICENAME2string(name)
       << "\"}";
    return ss.str();
}

void NetworkIdentity::set(
	const DEVADDR &addr,
	const DEVICEID &value
)
{
	memmove(&devaddr.u, &addr.u, sizeof(DEVADDR));
	memmove(&activation, &value.id.activation, sizeof(activation));
	memmove(&deviceclass, &value.id.deviceclass, sizeof(deviceclass));
	memmove(&devEUI, &value.id.devEUI, sizeof(DEVEUI));
	memmove(&nwkSKey.c, &value.id.nwkSKey.c, sizeof(KEY128));
	memmove(&appSKey.c, &value.id.appSKey.c, sizeof(KEY128));
	memmove(&appEUI, &value.id.appEUI, sizeof(DEVEUI));
	memmove(&appKey.c, &value.id.appKey.c, sizeof(KEY128));
	memmove(&nwkKey.c, &value.id.nwkKey.c, sizeof(KEY128));
	devNonce = value.id.devNonce;
	memmove(&joinNonce, &value.id.joinNonce, sizeof(JOINNONCE));
    token = value.id.token;
    region = value.id.region;
    subRegion = value.id.subRegion;
	memmove(&name, &value.id.name, sizeof(DEVICENAME));
	memmove(&version, &value.id.version, sizeof(LORAWAN_VERSION));
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
    switch (((MHDR*) buf)->f.mtype) {
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
            return calculateMICFrmPayload(b, (unsigned char) size,
                ((FHDR *) (b + 1))->fcnt, 0, identity.devaddr, identity.nwkSKey
            );
        }
        case MTYPE_UNCONFIRMED_DATA_DOWN:
        case MTYPE_CONFIRMED_DATA_DOWN: {
            if (size < SIZE_MHDR + SIZE_FHDR )
                return 0;
            return calculateMICFrmPayload(b, (unsigned char) size,
                ((FHDR *) (b + 1))->fcnt, 1, identity.devaddr, identity.nwkSKey
            );
        }
        default:
            // case MTYPE_PROPRIETARYRADIO:
            break;
    }
    return 0;
}
