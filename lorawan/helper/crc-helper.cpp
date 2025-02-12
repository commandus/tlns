#include "lorawan/helper/crc-helper.h"

/**
 * @see https://crccalc.com/
 * Poly     Init    RefIn   RefOut  XorOu
 * 0x1021   0x0000  false   false   0x0000
 * @param data
 * @param size
 * @return
 */
uint16_t crc16xmodem(
    const uint8_t *data,
    size_t size
) {
    uint16_t r = 0;
    for (unsigned int i = 0; i < size; ++i) {
        r ^= (uint16_t) data[i] << 8;
        for (int j = 0; j < 8; ++j) {
            r = (r & 0x8000) ? (r << 1) ^ 0x1021 : (r << 1);
        }
    }
    return r;
}

/**
 * @see https://crccalc.com/
 * Poly     Init    RefIn   RefOut  XorOu
 * 0x8005   0xffff  true    true   0x0000
 * @param data
 * @param size
 * @return
 */
uint16_t crc16modbus(
    const uint8_t *data,
    size_t size
) {
    uint16_t r = 0xffff;
    for (auto i = 0; i < size; ++i) {
        r ^= (uint16_t) data[i];
        for (int j = 0; j < 8; ++j) {
            if (r & 1)
                r = (r >> 1) ^ 0xA001;
            else
                r = (r >> 1);
        }
    }
    return r;
}
