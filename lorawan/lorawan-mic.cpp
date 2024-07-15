#include "lorawan/lorawan-mic.h"

#include <cstring>
#include "system/crypto/aes.h"
#include "system/crypto/cmac.h"
#include "lorawan/helper/aes-const.h"

static uint32_t calculateMICRev103(
	const unsigned char *data,
	const unsigned char size,
	const unsigned int frameCounter,
	const unsigned char direction,
	const DEVADDR &devAddr,
	const KEY128 &key
)
{
	unsigned char blockB[16];
	// blockB
	blockB[0] = 0x49;
    // 4.4.1 ConfFCnt Network Server and the ACK bit of the downlink frame is set, meaning this frame is acknowledging an uplink “confirmed” frame,
    // then ConfFCnt is the frame counter value modulo 2^16 of the “confirmed” uplink frame that
    // is being acknowledged. In all other cases ConfFCnt = 0x0000.
	blockB[1] = 0x00;
	blockB[2] = 0x00;
	blockB[3] = 0x00;
	blockB[4] = 0x00;

	blockB[5] = direction;

	blockB[6] = devAddr.c[0];
	blockB[7] = devAddr.c[1];
	blockB[8] = devAddr.c[2];
	blockB[9] = devAddr.c[3];

	blockB[10] = (frameCounter & 0x00FF);
	blockB[11] = ((frameCounter >> 8) & 0x00FF);

	blockB[12] = 0x00; // Frame counter upper bytes
	blockB[13] = 0x00;

	blockB[14] = 0x00;
	blockB[15] = size;

	aes_context aesContext;
	memset(aesContext.ksch, '\0', KSCH_SIZE);
    aes_set_key(key.c, sizeof(KEY128), &aesContext);

    AES_CMAC_CTX aesCmacCtx;
    AES_CMAC_Init(&aesCmacCtx);
	AES_CMAC_SetKey(&aesCmacCtx, key.c);
	AES_CMAC_Update(&aesCmacCtx, blockB, sizeof(blockB));
	AES_CMAC_Update(&aesCmacCtx, data, size);
	uint8_t mic[16];
	AES_CMAC_Final(mic, &aesCmacCtx);
    return (uint32_t) ((uint32_t) mic[3] << 24 | (uint32_t)mic[2] << 16 | (uint32_t)mic[1] << 8 | (uint32_t)mic[0] );
}

/**
 * Calculate MAC Frame Payload Encryption message integrity code
 * @see 4.3.3 MAC Frame Payload Encryption (FRMPayload)
 * message integrity code
 * B0
 * 1    4       1   4       4(3+1)       1 1
 * 0x49 0 0 0 0 Dir DevAddr frameCounter 0 Len(msg)
 * cmac = aes128_cmac(NwkSKey, B0 | msg)
 * MIC = cmac[0..3]
 *
 * 401111111100000001a1a46ff045b570
 *   DEV-ADDR  FRAMCN  PAYLOA
 * MH        FC(1.0) FP      MIC
 *           FRAME-CN (1.1)
 *
 * MH MAC header (40)
 * FC Frame control
 * CN Frame counter
 * FP Frame port
 */
uint32_t calculateMICFrmPayload(
	const unsigned char *data,
	unsigned char size,
	unsigned int frameCounter,
	unsigned char direction,
	const DEVADDR &devAddr,
	const KEY128 &key
)
{
	return calculateMICRev103(
		data,
		size,
		frameCounter,
		direction,
		devAddr,
		key
	);
}

/**
 * Calculate ReJoin Request MIC
 * @see 6.2.5 Join-request frame
 */
uint32_t calculateMICReJoinRequest(
    const JOIN_REQUEST_HEADER *header,
    const KEY128 &key,
    uint8_t rejoinType
) {
    aes_context aesContext;
    memset(aesContext.ksch, '\0', KSCH_SIZE);
    aes_set_key(key.c, sizeof(KEY128), &aesContext);

    AES_CMAC_CTX aesCmacCtx;
    AES_CMAC_Init(&aesCmacCtx);
    AES_CMAC_SetKey(&aesCmacCtx, key.c);
    AES_CMAC_Update(&aesCmacCtx, (const uint8_t *) header, 1 + sizeof(JOIN_REQUEST_FRAME));
    uint8_t mic[16];
    AES_CMAC_Final(mic, &aesCmacCtx);
    return (uint32_t) ((uint32_t)mic[3] << 24 | (uint32_t)mic[2] << 16 | (uint32_t)mic[1] << 8 | (uint32_t)mic[0] );
}

uint32_t calculateMICJoinRequest(
    const JOIN_REQUEST_HEADER *header,
    const KEY128 &key
) {
    return calculateMICReJoinRequest(header, key, 0xff);
}

/**
 * Calculate Join Response MIC OptNeg is unset (version 1.0)
 * MHDR | JoinNonce | NetID | DevAddr | DLSettings |RxDelay | [CFList]
 * @see 6.2.3 Join-accept message
 * @param frame Join Response message
 * @param key networrk key
 */
uint32_t calculateMICJoinResponse(
    const JOIN_ACCEPT_FRAME &frame,
    const KEY128 &key
) {
    aes_context aesContext;
    memset(aesContext.ksch, '\0', KSCH_SIZE);
    aes_set_key(key.c, sizeof(KEY128), &aesContext);

    AES_CMAC_CTX aesCmacCtx;
    AES_CMAC_Init(&aesCmacCtx);
    AES_CMAC_SetKey(&aesCmacCtx, key.c);
    AES_CMAC_Update(&aesCmacCtx, (const uint8_t *) &frame, 1 + sizeof(JOIN_ACCEPT_FRAME_HEADER));
    uint8_t mic[16];
    AES_CMAC_Final(mic, &aesCmacCtx);
    return (uint32_t) ((uint32_t)mic[3] << 24 | (uint32_t)mic[2] << 16 | (uint32_t)mic[1] << 8 | (uint32_t)mic[0] );
}

 /**
  * Calculate Join Response MIC OptNeg is set (version 1.1)
  * JoinReqType | JoinEUI | DevNonce | MHDR | JoinNonce | NetID | DevAddr | DLSettings |RxDelay | [CFList]
  * @see 6.2.3 Join-accept message
  * @param frame Join Accept frame
  * @param joinEUI Join EUI
  * @param devNonce Device nonce
  * @param key Key
  * @param rejoinType Type of rejoin
  * @return MIC
  */
uint32_t calculateOptNegMICJoinResponse(
    const JOIN_ACCEPT_FRAME &frame,
    const DEVEUI &joinEUI,
    const DEVNONCE &devNonce,
    const KEY128 &key,
    uint8_t rejoinType
) {
    // JoinReqType- 1, EUI- 8, DevNonce- 2, MHDR- 1, JOIN_ACCEPT_FRAME_HEADER- 12 = 24 bytes
    uint8_t d[24];
    // Join-request    0xFF, Rejoin-request type 0- 0x00,  Rejoin-request type 1- 0x01, Rejoin-request type 2- 0x02
    d[0] = rejoinType;   // JoinReqType 0xFF- join, 0, 1- rejoin
    // JoinEUI
    memmove(&(d[1]), &joinEUI, sizeof(DEVEUI)); // + 8
    // DevNonce
    memmove(&(d[9]), &devNonce, sizeof(DEVNONCE)); // + 2
    // same as OptNeg unset (version 1.0)
    memmove(&(d[10]), &frame.hdr.joinNonce, 1 + sizeof(JOIN_ACCEPT_FRAME_HEADER));

    aes_context aesContext;
    memset(aesContext.ksch, '\0', KSCH_SIZE);
    aes_set_key(key.c, sizeof(KEY128), &aesContext);

    AES_CMAC_CTX aesCmacCtx;
    AES_CMAC_Init(&aesCmacCtx);
    AES_CMAC_SetKey(&aesCmacCtx, key.c);
    AES_CMAC_Update(&aesCmacCtx, (const uint8_t *) &d, 1 + sizeof(d));
    uint8_t mic[16];
    AES_CMAC_Final(mic, &aesCmacCtx);
    return (uint32_t) ((uint32_t)mic[3] << 24 | (uint32_t)mic[2] << 16 | (uint32_t)mic[1] << 8 | (uint32_t)mic[0] );
}
