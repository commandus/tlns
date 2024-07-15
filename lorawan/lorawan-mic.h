#ifndef LORAWAN_MIC_H
#define LORAWAN_MIC_H

#include "lorawan/lorawan-types.h"

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
 * @param data message buffer
 * @param size message buffer size
 * @param frameCounter
 * @param direction 1- downlink
 * @param devAddr device address
 * @param key network session key
 * @return MIC
 */
uint32_t calculateMICFrmPayload(
	const unsigned char *data,
	unsigned char size,
	unsigned int frameCounter,
	unsigned char direction,
	const DEVADDR &devAddr,
	const KEY128 &key
);

/**
 * Calculate ReJoin Request MIC
 * @see 6.2.5 Join-request frame
 *
 * @param header
 * @param key network session key
 * @param rejoinType 0 or 2
 * @return MIC
 */
uint32_t calculateMICReJoinRequest(
    const JOIN_REQUEST_HEADER *header,
    const KEY128 &key,
    uint8_t rejoinType
);

uint32_t calculateMICJoinRequest(
    const JOIN_REQUEST_HEADER *header,
    const KEY128 &key
);

/**
 * Calculate Join Response MIC OptNeg is unset (version 1.0)
 * MHDR | JoinNonce | NetID | DevAddr | DLSettings |RxDelay | [CFList]
 * @see 6.2.3 Join-accept message
 */
uint32_t calculateMICJoinResponse(
    const JOIN_ACCEPT_FRAME &frame,
    const KEY128 &key
);

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
);

#endif //LORAWAN_MIC_H
