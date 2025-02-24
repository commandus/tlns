/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Functions used to handle LoRa concentrator SX1255/SX1257 radios.

License: Revised BSD License, see LICENSE.Semtech.txt file include in the project
*/

/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

#include "platform-win.h"

#include <stdint.h>     /* C99 types */

#include "sx125x_com.h"

int sx125x_com_r(lgw_com_type_t com_type, void *com_target, uint8_t spi_mux_target, uint8_t address, uint8_t *data) {
    int com_stat;

    /* Check input parameters */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    switch (com_type) {
        case LGW_COM_SPI:
            com_stat = sx125x_spi_r(com_target, spi_mux_target, address, data);
            break;
        case LGW_COM_USB:
            ERROR_PRINTF("ERROR: USB COM type is not supported for sx125x\n");
            return -1;
        default:
            ERROR_PRINTF("ERROR: wrong communication type (SHOULD NOT HAPPEN)\n");
            return -1;
    }

    return com_stat;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int sx125x_com_w(lgw_com_type_t com_type, void *com_target, uint8_t spi_mux_target, uint8_t address, uint8_t data) {
    int com_stat;

    /* Check input parameters */
    CHECK_NULL(com_target);

    switch (com_type) {
        case LGW_COM_SPI:
            com_stat = sx125x_spi_w(com_target, spi_mux_target, address, data);
            break;
        case LGW_COM_USB:
            ERROR_PRINTF("ERROR: USB COM type is not supported for sx125x\n");
            return -1;
        default:
            ERROR_PRINTF("ERROR: wrong communication type (SHOULD NOT HAPPEN)\n");
            return -1;
    }

    return com_stat;
}

/* --- EOF ------------------------------------------------------------------ */
