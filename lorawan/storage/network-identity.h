#ifndef NETWORK_IDENTITY_H_
#define NETWORK_IDENTITY_H_	1

#include <string>
#include "lorawan/lorawan-types.h"

class DeviceId;

/**
 * Section 6.3 Activating an end-device by personalization 
 * - DevAddr ->
 * - FNwkSIntKey
 * - SNwkSIntKey shared session key ->
 * - NwkSEncKey
 * - AppSKey private key ->
 * are directly stored into the end-device
*/
class NetworkIdentity {
private:
public:
	// key
	DEVADDR devaddr;		///< network address
	// value
	ACTIVATION activation;	///< activation type: ABP or OTAA
	DEVICECLASS deviceclass;
	DEVEUI devEUI;			///< device identifier
	KEY128 nwkSKey;			///< shared session key
	KEY128 appSKey;			///< private key
	LORAWAN_VERSION version;
	// OTAA
	DEVEUI appEUI;			///< OTAA application identifier
	KEY128 appKey;			///< OTAA application private key
    KEY128 nwkKey;          ///< OTAA network key
    DEVNONCE devNonce{};      ///< last device nonce
	JOINNONCE joinNonce;    ///< last Join nonce
	// added for searching
	DEVICENAME name;
	NetworkIdentity();
	NetworkIdentity(const DEVADDR &a, const DEVICEID &id);
    explicit NetworkIdentity(const DEVICEID &id);
	void set(const DEVADDR &addr, const DEVICEID &value);
	std::string toString() const;
    std::string toJsonString() const;
};

#endif
