/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Basic driver for ST ts751 temperature sensor

License: Revised BSD License, see LICENSE.Semtech.txt file include in the project
*/


/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

#include <stdint.h>     /* C99 types */
#include <stdbool.h>    /* bool type */

#include "platform-win.h"

#include "loragw_stts751.h"

#define STTS751_REG_TEMP_H      0x00
#define STTS751_REG_STATUS      0x01
#define STTS751_STATUS_TRIPT    BIT(0)
#define STTS751_STATUS_TRIPL    BIT(5)
#define STTS751_STATUS_TRIPH    BIT(6)
#define STTS751_REG_TEMP_L      0x02
#define STTS751_REG_CONF        0x03
#define STTS751_CONF_RES_MASK   0x0C
#define STTS751_CONF_RES_SHIFT  2
#define STTS751_CONF_EVENT_DIS  BIT(7)
#define STTS751_CONF_STOP       BIT(6)
#define STTS751_REG_RATE        0x04
#define STTS751_REG_HLIM_H      0x05
#define STTS751_REG_HLIM_L      0x06
#define STTS751_REG_LLIM_H      0x07
#define STTS751_REG_LLIM_L      0x08
#define STTS751_REG_TLIM        0x20
#define STTS751_REG_HYST        0x21
#define STTS751_REG_SMBUS_TO    0x22

#define STTS751_REG_PROD_ID     0xFD
#define STTS751_REG_MAN_ID      0xFE
#define STTS751_REG_REV_ID      0xFF

#define STTS751_0_PROD_ID       0x00
#define STTS751_1_PROD_ID       0x01
#define ST_MAN_ID               0x53

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

int stts751_configure(int i2c_fd, uint8_t i2c_addr) {
#if 0
    int err;
    uint8_t val;

    /* Check Input Params */
    if (i2c_fd <= 0) {
        ERROR_PRINTF("ERROR: invalid I2C file descriptor\n");
        return LGW_I2C_ERROR;
    }

    INFO_PRINTF("configuring STTS751 temperature sensor on 0x%02X...\n", i2c_addr);

    /* Get product ID  and test which sensor is mounted */
    err = i2c_linuxdev_read(i2c_fd, i2c_addr, STTS751_REG_PROD_ID, &val);
    if (err != 0) {
        ERROR_PRINTF("ERROR: failed to read I2C device 0x%02X (err=%i)\n", i2c_addr, err);
        return LGW_I2C_ERROR;
    }
    switch (val) {
        case STTS751_0_PROD_ID:
            INFO_PRINTF("Product ID: STTS751-0\n");
            break;
        case STTS751_1_PROD_ID:
            INFO_PRINTF("Product ID: STTS751-1\n");
            break;
        default:
            ERROR_PRINTF("ERROR: Product ID: UNKNOWN\n");
            return LGW_I2C_ERROR;
    }

    /* Get Manufacturer ID */
    err = i2c_linuxdev_read(i2c_fd, i2c_addr, STTS751_REG_MAN_ID, &val);
    if (err != 0) {
        ERROR_PRINTF("ERROR: failed to read I2C device 0x%02X (err=%i)\n", i2c_addr, err);
        return LGW_I2C_ERROR;
    }
    if (val != ST_MAN_ID) {
        ERROR_PRINTF("ERROR: Manufacturer ID: UNKNOWN\n");
        return LGW_I2C_ERROR;
    } else {
        INFO_PRINTF("Manufacturer ID: 0x%02X\n", val);
    }

    /* Get revision number */
    err = i2c_linuxdev_read(i2c_fd, i2c_addr, STTS751_REG_REV_ID, &val);
    if (err != 0) {
        ERROR_PRINTF("ERROR: failed to read I2C device 0x%02X (err=%i)\n", i2c_addr, err);
        return LGW_I2C_ERROR;
    }
    INFO_PRINTF("Revision number: 0x%02X\n", val);

    /* Set conversion resolution to 12 bits */
    err = i2c_linuxdev_write(i2c_fd, i2c_addr, STTS751_REG_CONF, 0x8C); /* TODO: do not hardcode the whole byte */
    if (err != 0) {
        ERROR_PRINTF("ERROR: failed to write I2C device 0x%02X (err=%i)\n", i2c_addr, err);
        return LGW_I2C_ERROR;
    }

    /* Set conversion rate to 1 / second */
    err = i2c_linuxdev_write(i2c_fd, i2c_addr, STTS751_REG_RATE, 0x04);
    if (err != 0) {
        ERROR_PRINTF("ERROR: failed to write I2C device 0x%02X (err=%i)\n", i2c_addr, err);
        return LGW_I2C_ERROR;
    }
#endif
    return LGW_I2C_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int stts751_get_temperature(int i2c_fd, uint8_t i2c_addr, float * temperature) {
#if 0
    int err;
    uint8_t high_byte, low_byte;
    int8_t h;

    /* Check Input Params */
    if (i2c_fd <= 0) {
        ERROR_PRINTF("ERROR: invalid I2C file descriptor\n");
        return LGW_I2C_ERROR;
    }

    /* Read Temperature LSB */
    err = i2c_linuxdev_read(i2c_fd, i2c_addr, STTS751_REG_TEMP_L, &low_byte);
    if (err != 0) {
        ERROR_PRINTF("ERROR: failed to read I2C device 0x%02X (err=%i)\n", i2c_addr, err);
        return LGW_I2C_ERROR;
    }

    /* Read Temperature MSB */
    err = i2c_linuxdev_read(i2c_fd, i2c_addr, STTS751_REG_TEMP_H, &high_byte);
    if (err != 0) {
        ERROR_PRINTF("ERROR: failed to read I2C device 0x%02X (err=%i)\n", i2c_addr, err);
        return LGW_I2C_ERROR;
    }

    h = (int8_t)high_byte;
    *temperature = ((h << 8) | low_byte) / 256.0;

    DEBUG_PRINTF("Temperature: %f C (h:0x%02X l:0x%02X)\n", *temperature, high_byte, low_byte);
#endif
    return LGW_I2C_SUCCESS;
}

/* --- EOF ------------------------------------------------------------------ */
