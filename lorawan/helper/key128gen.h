#include <cstddef>
#include <cinttypes>

#include "lorawan/lorawan-types.h"

/*
 * Step 1. Get passphrase (about 10-20 bytes long)
 * Step 2. Generate 128 bit long "master key" by the passphrase
 * Step 3. Generate 64 bit long EUI by the "master key" and "key number" = 1
 * Step 4. Generate 128 bit long NWK key by the "master key" and "key number" = 2
 * Step 5. Generate 128 bit long APP key by the "master key" and "key number" = 3
 */

enum KEY_NUMBER {
    KEY_NUMBER_EUI  =   1,
    KEY_NUMBER_NWK  =   2,
    KEY_NUMBER_APP  =   3
};

/**
 * Generate EUI
 * @param retVal return 8 bytes long EUI
 * @param keyNumber 0..n
 * @param key 128bit key
 * @param devAddr address
 */
void euiGen(
    uint8_t *retVal,
    uint32_t keyNumber,
    uint8_t *key,
    uint32_t devAddr
);

/**
 * Generate 128bit key
 * @param retVal return 128 bits (16 bytes) key
 * @param keyNumber 0..n
 * @param key "master key"
 * @param devAddr device address
 * @return
 */
uint8_t* keyGen(
    uint8_t* retVal,
    uint32_t keyNumber,
    uint8_t* key,
    uint32_t devAddr
);

/**
 * Generate pseudo-random 128 bit long key
 * @param retVal return 128 bits (16 bytes) "master key"
 * @return 128 bit long key
 */
uint8_t* rnd2key(
    uint8_t* retVal
);

/**
 * Generate "master key"
 * @param retVal return 128 bits (16 bytes) "master key"
 * @param phrase passphrase
 * @param size passphrase length
 * @return pointer to generated "master key"
 */
uint8_t* phrase2key(
    uint8_t* retVal,
    const char* phrase,
    size_t size
);

/*
 * Generates session keys 1)- network session key, 2)- application session key
 * @see LoRaWAN Specification v1.0 6.2.5 Join-accept message
 * @see LoRaMacJoinComputeSKeys() https://os.mbed.com/teams/Semtech/code/LoRaWAN-lib//file/2426a05fe29e/LoRaMacCrypto.cpp/
 *
 * NwkSKey = aes128_encrypt(AppKey, 0x01 | AppNonce | NetID | DevNonce | pad16)
 * AppSKey = aes128_encrypt(AppKey, 0x02 | AppNonce | NetID | DevNonce | pad16)
 * 0x01 | 0x02  1 bytes
 * AppNonce     3 bytes
 * NetID        3 bytes
 * DevNonce     2 bytes
 * 0            7 zeroes
 */
uint8_t* sessionKeyGen(
    uint8_t* retVal,
    KEY128& key,
    uint8_t keyType,	// 1 byte network: 1, application: 2
    APPNONCE appNonce,	// 3 bytes
    NETID netId,		// 3 bytes
    DEVNONCE& devNonce	// 2 bytes
);
