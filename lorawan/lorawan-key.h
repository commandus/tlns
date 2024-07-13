#ifndef LORAWAN_KEY_H
#define LORAWAN_KEY_H

#include "lorawan/lorawan-types.h"

// The network session keys are derived from the NwkKey:
// FNwkSIntKey = aes128_encrypt(NwkKey, 0x01 | JoinNonce | JoinEUI | DevNonce | pad 16 )
// SNwkSIntKey = aes128_encrypt(NwkKey, 0x03 | JoinNonce | JoinEUI | DevNonce | pad 16 )
// NwkSEncKey = aes128_encrypt(NwkKey, 0x04 | JoinNonce | JoinEUI | DevNonce | pad 16 )

/**
 * Derive JSIntKey used in Accept Join response in MIC calculation
 * @param retval return derived key
 * @param nwkKey NwkKey network key
 * @param devEUI Device EUI
 */
void deriveJSIntKey(
    KEY128 &retval,
    const KEY128 &nwkKey,
    const DEVEUI &devEUI
);

/**
 * JSEncKey is used to encrypt the Join-Accept triggered by a Rejoin-Request
 * @param retval return derived key
 * @param nwkKey NwkKey network key
 * @param devEUI Device EUI
 */
void deriveJSEncKey(
    KEY128 &retval,
    const KEY128 &nwkKey,
    const DEVEUI &devEUI
);

/**
 * DeriveSessionKey
 * @param retval
 * @param tag
 * @param key
 * @param devEUI
 * @param joinNonce
 * @param devNonce
 */
static void deriveDevSessionKey(
    KEY128 &retval,
    uint8_t tag,
    const KEY128 &key,
    const DEVEUI &devEUI,
    const JOINNONCE &joinNonce,
    const DEVNONCE &devNonce
);

// OptNeg is set
// FNwkSIntKey = aes128_encrypt(NwkKey, 0x01 | JoinNonce | JoinEUI | DevNonce | pad 16 )
// SNwkSIntKey = aes128_encrypt(NwkKey, 0x03 | JoinNonce | JoinEUI | DevNonce | pad 16 )
// NwkSEncKey = aes128_encrypt(NwkKey, 0x04 | JoinNonce | JoinEUI | DevNonce | pad 16 )

void deriveOptNegFNwkSIntKey(
    KEY128 &retval,
    const KEY128 &key,
    const DEVEUI &joinEUI,
    const JOINNONCE &joinNonce,
    const DEVNONCE &devNonce
);

void deriveOptNegSNwkSIntKey(
    KEY128 &retval,
    const KEY128 &key,
    const DEVEUI &joinEUI,
    const JOINNONCE &joinNonce,
    const DEVNONCE &devNonce
);

void deriveOptNegNwkSEncKey(
    KEY128 &retval,
    const KEY128 &key,
    const DEVEUI &joinEUI,
    const JOINNONCE &joinNonce,
    const DEVNONCE &devNonce
);

// AppSKey = aes128_encrypt(NwkKey, 0x02 | JoinNonce | NetID | DevNonce | pad 161 )
// FNwkSIntKey = aes128_encrypt(NwkKey, 0x01 | JoinNonce | NetID | DevNonce | pad 16 )
// SNwkSIntKey = NwkSEncKey = FNwkSIntKey.
/**
 * OptNeg is unset
 * Derive AppSKey
 * @param retval derived key
 * @param key key
 * @param netId Network identifier
 * @param joinNonce Join nonce
 * @param devNonce Device nonce
 */
void deriveAppSKey(
    KEY128 &retval,
    const KEY128 &key,
    const NETID &netId,
    const JOINNONCE &joinNonce,
    const DEVNONCE &devNonce
);

/**
 * OptNeg is unset
 * Derive SNwkSIntKey = NwkSEncKey = FNwkSIntKey.
 * @param retval derived key
 * @param key key
 * @param netId network identifier
 * @param joinNonce Join nonce
 * @param devNonce Device nonce
 */
void deriveFNwkSIntKey(
    KEY128 &retval,
    const KEY128 &key,
    const NETID &netId,
    const JOINNONCE &joinNonce,
    const DEVNONCE &devNonce
);

#endif //LORAWAN_KEY_H
