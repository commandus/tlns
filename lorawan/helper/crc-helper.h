#ifndef LORAWAN_STORAGE_CRC_HELPER_H
#define LORAWAN_STORAGE_CRC_HELPER_H

#include <cstddef>
#include <cinttypes>

/**
 * @param data
 * @param size
 * @return
 */
uint16_t crc16xmodem(
    const uint8_t *data,
    size_t size
);

/**
 * Modicon Modbus Protocol Reference Guide, Modicon Inc., June, 1996. 367
 * @see https://modbus.org/docs/PI_MBUS_300.pdf
 * @param data
 * @param size
 * @return
 */
uint16_t crc16modbus(
    const uint8_t *data,
    size_t size
);

#endif
