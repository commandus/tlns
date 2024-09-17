#ifndef AES_HELPER_H
#define AES_HELPER_H

#include <string>
#include "lorawan/lorawan-types.h"

#define LORAWAN_UPLINK 1
#define LORAWAN_DOWNLINK  0

/**
 * @see 4.3.3 MAC Frame Payload Encryption (FRMPayload)
 * @see https://os.mbed.com/teams/Semtech/code/LoRaWAN-lib//file/2426a05fe29e/LoRaMacCrypto.cpp/
 */
void encryptPayload(
    void *payload,
    size_t size,
    unsigned int frameCounter,
    unsigned char direction,
    const DEVADDR &devAddr,
    const KEY128 &appSKey
);

void encryptPayloadString(
    std::string &payload,
    unsigned int frameCounter,
    unsigned char direction,
    const DEVADDR &devAddr,
    const KEY128 &appSKey
);

#define decryptPayload(payload, size, frameCounter, direction, devAddr,appSKey) encryptPayload(payload, size, frameCounter, direction, devAddr,appSKey)
#define encryptPayloadString(payload, frameCounter, direction, devAddr,appSKey) encryptPayload((void *) payload.c_str(), payload.size(), frameCounter, direction, devAddr,appSKey)
#define decryptPayloadString(payload, frameCounter, direction, devAddr,appSKey) encryptPayloadString(payload, frameCounter, direction, devAddr,appSKey)

/**
 * Decrypt Join Accept LoRaWAN message
 * @see 6.2.3 Join-accept message
 */
void decryptJoinAccept(
    void *payload,
    size_t size,
    const KEY128 &key
);

void decryptJoinAcceptString(
    const std::string &payload,
    const KEY128 &key
);

/**
 * Encrypt Join-Accept response
 * aes128_decrypt(NwkKey or JSEncKey, JoinNonce | NetID | DevAddr | DLSettings | RxDelay | CFList | MIC).
 * @param frame return value
 * @param key NwkKey or JSEncKey
 */
void encryptJoinAcceptResponse(
    JOIN_ACCEPT_FRAME &frame,
    const KEY128 &key   // NwkKey or JSEncKey
);

/**
 * Encrypt Join-Accept with CFList response
 * aes128_decrypt(NwkKey or JSEncKey, JoinNonce | NetID | DevAddr | DLSettings | RxDelay | CFList | MIC).
 * @param frame return value
 * @param key NwkKey or JSEncKey
 */
void encryptJoinAcceptCFListResponse(
    JOIN_ACCEPT_FRAME_CFLIST &frame,
    const KEY128 &key   // NwkKey or JSEncKey
);

#endif // AES_HELPER_H
