#include "lorawan/lorawan-key.h"

#include <cstring>
#include "system/crypto/aes.h"
#include "system/crypto/cmac.h"
#include "lorawan/helper/aes-const.h"

/**
 * JSEncKey is used to encrypt the Join-Accept triggered by a Rejoin-Request
 * @param retval return derived key
 * @param nwkKey NwkKey network key
 * @param value Device EUI
 * @param size size
 */
static void deriveKeyBlock(
    KEY128 &retval,
    const KEY128 &nwkKey,
    const void *value,
    size_t size
)
{
    aes_context aesContext;
    memset(aesContext.ksch, '\0', KSCH_SIZE);
    aes_set_key(nwkKey.c, sizeof(KEY128), &aesContext);

    AES_CMAC_CTX aesCmacCtx;
    AES_CMAC_Init(&aesCmacCtx);
    AES_CMAC_SetKey(&aesCmacCtx, nwkKey.c);
    AES_CMAC_Update(&aesCmacCtx, (const uint8_t *) &value, (std::uint32_t) size);
    AES_CMAC_Final(retval.c, &aesCmacCtx);
}

// The network session keys are derived from the NwkKey:
// FNwkSIntKey = aes128_encrypt(NwkKey, 0x01 | JoinNonce | JoinEUI | DevNonce | pad 16 )
// SNwkSIntKey = aes128_encrypt(NwkKey, 0x03 | JoinNonce | JoinEUI | DevNonce | pad 16 )
// NwkSEncKey = aes128_encrypt(NwkKey, 0x04 | JoinNonce | JoinEUI | DevNonce | pad 16 )
/**
 * JSEncKey is used to encrypt the Join-Accept triggered by a Rejoin-Request
 * @param retval return derived key
 * @param tag Tag (first byte)
 * @param nwkKey NwkKey network key
 * @param value Device EUI
 * @param size size
 */
static void deriveKey(
    KEY128 &retval,
    uint8_t tag,
    const KEY128 &nwkKey,
    const void *value,
    const size_t size
)
{
    uint8_t block[16];
    block[0] = tag;
    memmove(&(block[1]), value, size);
    memset(&(block[9]), '\0', 7);
    deriveKeyBlock(retval, nwkKey, &block, sizeof(block));
}

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
)
{
    deriveKey(retval, 6, nwkKey, &devEUI, sizeof(DEVEUI));
}

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
)
{
    deriveKey(retval, 5, nwkKey, &devEUI, sizeof(DEVEUI));
}

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
)
{
    uint8_t block[16];
    block[0] = tag;
    memmove(&(block[1]), &joinNonce, sizeof(JOINNONCE));                // + 3 = 4
    memmove(&(block[1 + sizeof(joinNonce)]), &devEUI, sizeof(DEVEUI));  // + 8 = 12
    memmove(&(block[1 + sizeof(joinNonce) + sizeof(devEUI)]), &devNonce, sizeof(DEVNONCE)); // + 2 = 14
    memset(&(block[14]), '\0', 2);
    deriveKeyBlock(retval, key, &block, sizeof(block));
}

/**
 * DeriveSessionKey
 * @param retval
 * @param tag
 * @param key
 * @param netId
 * @param joinNonce
 * @param devNonce
 */
static void deriveNetSessionKey(
        KEY128 &retval,
        uint8_t tag,
        const KEY128 &key,
        const NETID &netId,
        const JOINNONCE &joinNonce,
        const DEVNONCE &devNonce
)
{
    uint8_t block[16];
    block[0] = tag;
    memmove(&(block[1]), &joinNonce, sizeof(JOINNONCE));                // + 3 = 4
    memmove(&(block[1 + sizeof(joinNonce)]), &netId.c, sizeof(NETID));    // + 3 = 7
    memmove(&(block[1 + sizeof(joinNonce) + sizeof(NETID)]), &devNonce, sizeof(DEVNONCE)); // + 2 = 9
    memset(&(block[9]), '\0', 7);
    deriveKeyBlock(retval, key, &block, sizeof(block));
}

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
)
{
    deriveDevSessionKey(retval, 1, key, joinEUI, joinNonce, devNonce);
}

void deriveOptNegSNwkSIntKey(
        KEY128 &retval,
        const KEY128 &key,
        const DEVEUI &joinEUI,
        const JOINNONCE &joinNonce,
        const DEVNONCE &devNonce
)
{
    deriveDevSessionKey(retval, 3, key, joinEUI, joinNonce, devNonce);
}

void deriveOptNegNwkSEncKey(
        KEY128 &retval,
        const KEY128 &key,
        const DEVEUI &joinEUI,
        const JOINNONCE &joinNonce,
        const DEVNONCE &devNonce
)
{
    deriveDevSessionKey(retval, 4, key, joinEUI, joinNonce, devNonce);
}

// AppSKey = aes128_encrypt(NwkKey, 0x02 | JoinNonce | NetID | DevNonce | pad 161 )
// FNwkSIntKey = aes128_encrypt(NwkKey, 0x01 | JoinNonce | NetID | DevNonce | pad 16 )
// SNwkSIntKey = NwkSEncKey = FNwkSIntKey.
/**
 * OptNeg is unset
 * Derive AppSKey
 * @param retval derived key
 * @param key key
 * @param netid Network identifier
 * @param joinNonce Join nonce
 * @param devNonce Device nonce
 */
void deriveAppSKey(
    KEY128 &retval,
    const KEY128 &key,
    const NETID &netId,
    const JOINNONCE &joinNonce,
    const DEVNONCE &devNonce
)
{
    deriveNetSessionKey(retval, 2, key, netId, joinNonce, devNonce);
}

/**
 * OptNeg is unset
 * Derive SNwkSIntKey = NwkSEncKey = FNwkSIntKey.
 * @param retval derived key
 * @param key key
 * @param netid network identifier
 * @param joinNonce Join nonce
 * @param devNonce Device nonce
 */
void deriveFNwkSIntKey(
        KEY128 &retval,
        const KEY128 &key,
        const NETID &netId,
        const JOINNONCE &joinNonce,
        const DEVNONCE &devNonce
)
{
    deriveNetSessionKey(retval, 1, key, netId, joinNonce, devNonce);
}
