/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2020 Semtech

Description:
    Basic driver for Analog AD5338R DAC.

License: Revised BSD License, see LICENSE.Semtech.txt file include in the project
*/

#include "platform-win.h"

#include <stdint.h>     /* C99 types */
#include <stdbool.h>    /* bool type */

#include "loragw_i2c.h"
#include "loragw_ad5338r.h"

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

int ad5338r_configure(int i2c_fd, uint8_t i2c_addr) {
    int err;
    uint8_t cmd_soft_reset[AD5338R_CMD_SIZE] = {0x69, 0x00, 0x00};
    uint8_t cmd_power_up_dn[AD5338R_CMD_SIZE] = {0x40, 0x00, 0x00};
    uint8_t cmd_internal_ref[AD5338R_CMD_SIZE] = {0x70, 0x00, 0x00};

    /* Check Input Params */
    if (i2c_fd <= 0) {
        ERROR_PRINTF("ERROR: invalid I2C file descriptor\n");
        return LGW_I2C_ERROR;
    }

    INFO_PRINTF("Configuring AD5338R DAC on 0x%02X...\n", i2c_addr);

    /* Sofwtare reset of DAC A & B */
    /* TODO: LSB data bytes seems not acknoledged by AD5338R, leading to an error if sent.
        Only send MSB data byte */
    err = i2c_linuxdev_write(i2c_fd, i2c_addr, cmd_soft_reset[0], cmd_soft_reset[1]);
    if (err != 0) {
        ERROR_PRINTF("ERROR: AD5338R software reset failed\n");
        return LGW_I2C_ERROR;
    }

    /* Normal operation */
    err = ad5338r_write(i2c_fd, i2c_addr, cmd_power_up_dn);
    if (err != 0) {
        ERROR_PRINTF("ERROR: AD5338R failed to set to normal operation\n");
        return LGW_I2C_ERROR;
    }

    /* Internal reference ON */
    err = ad5338r_write(i2c_fd, i2c_addr, cmd_internal_ref);
    if (err != 0) {
        ERROR_PRINTF("ERROR: AD5338R failed to set internal reference ON\n");
        return LGW_I2C_ERROR;
    }

    INFO_PRINTF("AD5338R is configured\n");

    return LGW_I2C_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int ad5338r_write(int i2c_fd, uint8_t i2c_addr, uint8_t buf[AD5338R_CMD_SIZE]) {
    int err;

    /* Write AD5338R command buffer */
    err = i2c_linuxdev_write_buffer(i2c_fd, i2c_addr, buf, AD5338R_CMD_SIZE);
    if (err != 0) {
        ERROR_PRINTF("ERROR: failed to write AD5338R command\n");
        return LGW_I2C_ERROR;
    }

    return LGW_I2C_SUCCESS;
}
