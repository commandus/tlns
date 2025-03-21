/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Functions used to handle LoRa concentrator SX1250 radios.

License: Revised BSD License, see LICENSE.Semtech.txt file include in the project
*/


/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

#include "platform-win.h"

#include <stdint.h>     /* C99 types */
#include <string.h>     /* memcmp */

#include "loragw_aux.h"
#include "loragw_mcu.h"
#include "sx1250_usb.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

#define WAIT_BUSY_SX1250_MS  1

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

int sx1250_usb_w(void *com_target, uint8_t spi_mux_target, sx1250_op_code_t op_code, uint8_t *data, uint16_t size) {
    int usb_device;
    uint8_t command_size = size + 7; /* 5 bytes: REQ metadata, 2 bytes: RAW SPI frame */
    uint8_t *in_out_buf = alloca(command_size);
    int a;
    int i;

    /* check input variables */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    usb_device = *(int *)com_target;

    /* wait BUSY */
    wait_ms(WAIT_BUSY_SX1250_MS);

    /* prepare command */
    /* Request metadata */
    in_out_buf[0] = 0; /* Req ID */
    in_out_buf[1] = MCU_SPI_REQ_TYPE_READ_WRITE; /* Req type */
    in_out_buf[2] = MCU_SPI_TARGET_SX1302; /* MCU -> SX1302 */
    in_out_buf[3] = (uint8_t)((size + 2) >> 8); /* payload size + spi_mux_target + op_code */
    in_out_buf[4] = (uint8_t)((size + 2) >> 0); /* payload size + spi_mux_target + op_code */
    /* RAW SPI frame */
    in_out_buf[5] = spi_mux_target; /* SX1302 -> RADIO_A or RADIO_B */
    in_out_buf[6] = (uint8_t)op_code;
    for (i = 0; i < size; i++) {
        in_out_buf[i + 7] = data[i];
    }
    a = mcu_spi_write(usb_device, in_out_buf, command_size);

    /* determine return code */
    if (a != 0) {
        ERROR_PRINTF("ERROR: USB SX1250 WRITE FAILURE\n");
        return -1;
    } else {
#if DEBUG_USB == 1
        DEBUG_PRINTF("Note: USB SX1250 write success\n");
#endif
        return 0;
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int sx1250_usb_r(void *com_target, uint8_t spi_mux_target, sx1250_op_code_t op_code, uint8_t *data, uint16_t size) {
    int usb_device;
    uint8_t command_size = size + 7; /* 5 bytes: REQ metadata, 2 bytes: RAW SPI frame */
    uint8_t *in_out_buf = alloca(command_size);
    int a;
    int i;

    /* check input variables */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    usb_device = *(int *)com_target;

    /* wait BUSY */
    wait_ms(WAIT_BUSY_SX1250_MS);

    /* prepare command */
    /* Request metadata */
    in_out_buf[0] = 0; /* Req ID */
    in_out_buf[1] = MCU_SPI_REQ_TYPE_READ_WRITE; /* Req type */
    in_out_buf[2] = MCU_SPI_TARGET_SX1302; /* MCU -> SX1302 */
    in_out_buf[3] = (uint8_t)((size + 2) >> 8); /* payload size + spi_mux_target + op_code */
    in_out_buf[4] = (uint8_t)((size + 2) >> 0); /* payload size + spi_mux_target + op_code */
    /* RAW SPI frame */
    in_out_buf[5] = spi_mux_target; /* SX1302 -> RADIO_A or RADIO_B */
    in_out_buf[6] = (uint8_t)op_code;
    for (i = 0; i < size; i++) {
        in_out_buf[i + 7] = data[i];
    }
    a = mcu_spi_write(usb_device, in_out_buf, command_size);

    /* determine return code */
    if (a != 0) {
        ERROR_PRINTF("ERROR: USB SX1250 READ FAILURE\n");
        return -1;
    } else {
#if DEBUG_USB == 1
        DEBUG_PRINTF("Note: USB SX1250 read success\n");
#endif
        memcpy(data, in_out_buf + 7, size); /* remove the first bytes, keep only the payload */
        return 0;
    }
}

/* --- EOF ------------------------------------------------------------------ */
