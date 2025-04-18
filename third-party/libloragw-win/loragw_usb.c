/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2020 Semtech

Description:
    Host specific functions to address the LoRa concentrator registers through
    a USB interface.
    Single-byte read/write and burst read/write.

License: Revised BSD License, see LICENSE.Semtech.txt file include in the project
*/


/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */
#include "platform-win.h"

#include <stdint.h>     /* C99 types */
#include <stdbool.h>    /* bool type */
#include <stdlib.h>     /* malloc free */


#include <fcntl.h>      /* open */
#include <string.h>     /* strncmp */

#include "loragw_com.h"
#include "loragw_usb.h"
#include "loragw_mcu.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES  --------------------------------------------------- */

static lgw_com_write_mode_t _lgw_write_mode = LGW_COM_WRITE_MODE_SINGLE;
static uint8_t _lgw_spi_req_nb = 0;

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DEFINITION ----------------------------------------- */

/* configure serial interface to be read blocking or not*/
int set_blocking_windows(int fd, bool blocking) {
    return setSerialBlocking(fd, blocking) ? LGW_USB_SUCCESS : -1;
}

static int set_interface_attribs_windows(int fd, int speed) {
    setSerialSpeed(fd, speed);
    return set_blocking_windows(fd, false);
}

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

int lgw_usb_open(const char * com_path, void **com_target_ptr) {
    int *usb_device = NULL;
    char portname[50];
    int x;
    int fd;
    s_ping_info gw_info;
    s_status mcu_status;
    uint8_t data;
    ssize_t n;

    /*check input variables*/
    CHECK_NULL(com_target_ptr);

    usb_device = malloc(sizeof(int));
    if (usb_device == NULL) {
#ifdef DEBUG_USB
        DEBUG_PRINTF("ERROR : MALLOC FAIL\n");
#endif
        return LGW_USB_ERROR;
    }

    /* open tty port */
    sprintf(portname, "%s", com_path);
    fd = openSerial(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        ERROR_PRINTF("ERROR: failed to open COM port %s - %s\n", portname, strerror(errno));
    } else {
        INFO_PRINTF("Configuring TTY\n");
        x = set_interface_attribs_windows(fd, 115200);
        if (x != 0) {
            ERROR_PRINTF("ERROR: failed to configure COM port %s %d\n", portname, GetLastError());
            free(usb_device);
            return LGW_USB_ERROR;
        }

        /* flush tty port before setting it as blocking */
        INFO_PRINTF("Flushing TTY\n");
        do {
            n = read(fd, &data, 1);
            if (n > 0) {
                INFO_PRINTF("NOTE: flushing serial port (0x%2X)\n", data);
            }
        } while (n > 0);

        /* set tty port blocking */
        INFO_PRINTF("Setting TTY in blocking mode\n");
        x = set_blocking_windows(fd, true);
        if (x != 0) {
            ERROR_PRINTF("ERROR: failed to configure COM port %s\n", portname);
            free(usb_device);
            return LGW_USB_ERROR;
        }

        *usb_device = fd;
        *com_target_ptr = (void*)usb_device;

        /* Initialize pseudo-random generator for MCU request ID */
        srand(0);

        /* Check MCU version (ignore first char of the received version (release/debug) */
        INFO_PRINTF("Connect to MCU\n");
        if (mcu_ping(fd, &gw_info) != 0) {
            ERROR_PRINTF("ERROR: failed to ping the concentrator MCU\n");
            return LGW_USB_ERROR;
        }
        if (strncmp(gw_info.version + 1, mcu_version_string, sizeof mcu_version_string) != 0) {
            INFO_PRINTF("WARNING: MCU version mismatch (expected:%s, got:%s)\n", mcu_version_string, gw_info.version);
        }
        INFO_PRINTF("Concentrator MCU version is %s\n", gw_info.version);

        /* Get MCU status */
        if (mcu_get_status(fd, &mcu_status) != 0) {
            ERROR_PRINTF("ERROR: failed to getUplink status from the concentrator MCU\n");
            return LGW_USB_ERROR;
        }
        INFO_PRINTF("MCU status: sys_time:%u temperature:%.1foC\n", mcu_status.system_time_ms, mcu_status.temperature);

        /* Reset SX1302 */
        x  = mcu_gpio_write(fd, 0, 1, 1); /*   set PA1 : POWER_EN */
        x |= mcu_gpio_write(fd, 0, 2, 1); /*   set PA2 : SX1302_RESET active */
        x |= mcu_gpio_write(fd, 0, 2, 0); /* unset PA2 : SX1302_RESET inactive */
        /* Reset SX1261 (LBT / Spectral Scan) */
        x |= mcu_gpio_write(fd, 0, 8, 0); /*   set PA8 : SX1261_NRESET active */
        x |= mcu_gpio_write(fd, 0, 8, 1); /* unset PA8 : SX1261_NRESET inactive */
        if (x != 0) {
            ERROR_PRINTF("ERROR: failed to reset SX1302\n");
            free(usb_device);
            return LGW_USB_ERROR;
        }

        return LGW_USB_SUCCESS;
    }

    free(usb_device);
    return LGW_USB_ERROR;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* SPI release */
int lgw_usb_close(void *com_target) {
    int usb_device;
    int x, err = LGW_USB_SUCCESS;

    /* check input variables */
    CHECK_NULL(com_target);

    usb_device = *(int *)com_target;

    /* Reset SX1302 before closing */
    x  = mcu_gpio_write(usb_device, 0, 1, 1); /*   set PA1 : POWER_EN */
    x |= mcu_gpio_write(usb_device, 0, 2, 1); /*   set PA2 : SX1302_RESET active */
    x |= mcu_gpio_write(usb_device, 0, 2, 0); /* unset PA2 : SX1302_RESET inactive */
    /* Reset SX1261 (LBT / Spectral Scan) */
    x |= mcu_gpio_write(usb_device, 0, 8, 0); /*   set PA8 : SX1261_NRESET active */
    x |= mcu_gpio_write(usb_device, 0, 8, 1); /* unset PA8 : SX1261_NRESET inactive */
    if (x != 0) {
        ERROR_PRINTF("ERROR: failed to reset SX1302\n");
        err = LGW_USB_ERROR;
    }

    /* close file & deallocate file descriptor */
    x = close(usb_device);
    free(com_target);
    if (x != 0) {
        ERROR_PRINTF("ERROR: failed to close USB file\n");
        err = LGW_USB_ERROR;
    }

    /* determine return code */
    if (err != 0) {
        ERROR_PRINTF("ERROR: USB PORT FAILED TO CLOSE\n");
        return LGW_USB_ERROR;
    } else {
#ifdef DEBUG_USB
        DEBUG_PRINTF("Note: USB port closed\n");
#endif
        return LGW_USB_SUCCESS;
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Simple write */
int lgw_usb_w(void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t data) {
    return lgw_usb_wb(com_target, spi_mux_target, address, &data, 1);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Simple read */
int lgw_usb_r(void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t *data) {
    return lgw_usb_rb(com_target, spi_mux_target, address, data, 1);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Single Byte Read-Modify-Write */
int lgw_usb_rmw(void *com_target, uint16_t address, uint8_t offs, uint8_t leng, uint8_t data) {
    int usb_device;
    uint8_t command_size = 6;
    uint8_t *in_out_buf = alloca(command_size);
    int a = 0;

    /* check input variables */
    CHECK_NULL(com_target);

    usb_device = *(int *)com_target;
#ifdef DEBUG_USB
    DEBUG_PRINTF("==> RMW register @ 0x%04X, offs:%u leng:%u value:0x%02X\n", address, offs, leng, data);
#endif
    /* prepare frame to be sent */
    in_out_buf[0] = _lgw_spi_req_nb; /* Req ID */
    in_out_buf[1] = MCU_SPI_REQ_TYPE_READ_MODIFY_WRITE; /* Req type */
    in_out_buf[2] = (uint8_t)(address >> 8); /* Register address MSB */
    in_out_buf[3] = (uint8_t)(address >> 0); /* Register address LSB */
    in_out_buf[4] = ((1 << leng) - 1) << offs; /* Register bitmask */
    in_out_buf[5] = data << offs;

    if (_lgw_write_mode == LGW_COM_WRITE_MODE_BULK) {
        a = mcu_spi_store(in_out_buf, command_size);
        _lgw_spi_req_nb += 1;
    } else {
        a = mcu_spi_write(usb_device, in_out_buf, command_size);
    }

    /* determine return code */
    if (a != 0) {
        ERROR_PRINTF("ERROR: USB WRITE FAILURE\n");
        return -1;
    } else {
#ifdef DEBUG_USB
        DEBUG_PRINTF("Note: USB write success\n");
#endif
        return 0;
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Burst (multiple-byte) write */
int lgw_usb_wb(void *com_target, uint8_t spi_mux_target, uint16_t address, const uint8_t *data, uint16_t size) {
    int usb_device;
    uint16_t command_size = size + 8; /* 5 bytes: REQ metadata (MCU), 3 bytes: SPI header (SX1302) */
    uint8_t *in_out_buf = alloca(command_size);
    int i;
    int a = 0;

    /* check input parameters */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    usb_device = *(int *)com_target;

    /* prepare command */
    /* Request metadata */
    in_out_buf[0] = _lgw_spi_req_nb; /* Req ID */
    in_out_buf[1] = MCU_SPI_REQ_TYPE_READ_WRITE; /* Req type */
    in_out_buf[2] = MCU_SPI_TARGET_SX1302; /* MCU -> SX1302 */
    in_out_buf[3] = (uint8_t)((size + 3) >> 8); /* payload size + spi_mux_target + address */
    in_out_buf[4] = (uint8_t)((size + 3) >> 0); /* payload size + spi_mux_target + address */
    /* RAW SPI frame */
    in_out_buf[5] = spi_mux_target; /* SX1302 -> RADIO_A or RADIO_B */
    in_out_buf[6] = 0x80 | ((address >> 8) & 0x7F);
    in_out_buf[7] =        ((address >> 0) & 0xFF);
    for (i = 0; i < size; i++) {
        in_out_buf[i + 8] = data[i];
    }

    if (_lgw_write_mode == LGW_COM_WRITE_MODE_BULK) {
        a = mcu_spi_store(in_out_buf, command_size);
        _lgw_spi_req_nb += 1;
    } else {
        a = mcu_spi_write(usb_device, in_out_buf, command_size);
    }

    /* determine return code */
    if (a != 0) {
        ERROR_PRINTF("ERROR: USB WRITE BURST FAILURE\n");
        return -1;
    } else {
#ifdef DEBUG_USB
        DEBUG_PRINTF("Note: USB write burst success\n");
#endif
        return 0;
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Burst (multiple-byte) read */
int lgw_usb_rb(void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t *data, uint16_t size) {
    int usb_device;
    uint16_t command_size = size + 9;  /* 5 bytes: REQ metadata (MCU), 3 bytes: SPI header (SX1302), 1 byte: dummy*/
    uint8_t *in_out_buf = alloca(command_size);
    int i;
    int a = 0;

    /* check input parameters */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    usb_device = *(int *)com_target;

    /* prepare command */
    /* Request metadata */
    in_out_buf[0] = 0; /* Req ID */
    in_out_buf[1] = MCU_SPI_REQ_TYPE_READ_WRITE; /* Req type */
    in_out_buf[2] = MCU_SPI_TARGET_SX1302; /* MCU -> SX1302 */
    in_out_buf[3] = (uint8_t)((size + 4) >> 8); /* payload size + spi_mux_target + address + dummy byte */
    in_out_buf[4] = (uint8_t)((size + 4) >> 0); /* payload size + spi_mux_target + address + dummy byte */
    /* RAW SPI frame */
    in_out_buf[5] = spi_mux_target; /* SX1302 -> RADIO_A or RADIO_B */
    in_out_buf[6] = 0x00 | ((address >> 8) & 0x7F);
    in_out_buf[7] =        ((address >> 0) & 0xFF);
    in_out_buf[8] = 0x00; /* dummy byte */
    for (i = 0; i < size; i++) {
        in_out_buf[i + 9] = data[i];
    }

    if (_lgw_write_mode == LGW_COM_WRITE_MODE_BULK) {
        /* makes no sense to read in bulk mode, as we can't getUplink the result */
        ERROR_PRINTF("ERROR: USB READ BURST FAILURE - bulk mode is enabled\n");
        return -1;
    } else {
        a = mcu_spi_write(usb_device, in_out_buf, command_size);
    }

    /* determine return code */
    if (a != 0) {
        ERROR_PRINTF("ERROR: USB READ BURST FAILURE\n");
        return -1;
    } else {
#ifdef DEBUG_USB
        DEBUG_PRINTF("Note: USB read burst success\n");
#endif
        memcpy(data, in_out_buf + 9, size); /* remove the first bytes, keep only the payload */
        return 0;
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_usb_set_write_mode(lgw_com_write_mode_t write_mode) {
    if (write_mode >= LGW_COM_WRITE_MODE_UNKNOWN) {
        ERROR_PRINTF("ERROR: wrong write mode\n");
        return -1;
    }

   INFO_PRINTF("Setting USB write mode to %s\n", (write_mode == LGW_COM_WRITE_MODE_SINGLE) ? "SINGLE" : "BULK");

    _lgw_write_mode = write_mode;

    return 0;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_usb_flush(void *com_target) {
    int usb_device;
    int a = 0;

    /* Check input parameters */
    CHECK_NULL(com_target);
    if (_lgw_write_mode != LGW_COM_WRITE_MODE_BULK) {
        ERROR_PRINTF("ERROR: %s: cannot flush in single write mode\n", __FUNCTION__);
        return -1;
    }

    /* Restore single mode after flushing */
    _lgw_write_mode = LGW_COM_WRITE_MODE_SINGLE;

    if (_lgw_spi_req_nb == 0) {
        INFO_PRINTF("INFO: no SPI request to flush\n");
        return 0;
    }

    usb_device = *(int *)com_target;

    INFO_PRINTF("flushing USB write buffer\n");
    a = mcu_spi_flush(usb_device);
    if (a != 0) {
        ERROR_PRINTF("ERROR: Failed to flush USB write buffer\n");
    }

    /* reset the pending request number */
    _lgw_spi_req_nb = 0;

    return a;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

uint16_t lgw_usb_chunk_size(void) {
    return (uint16_t)LGW_USB_BURST_CHUNK;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_usb_get_temperature(void *com_target, float * temperature) {
    int usb_device;
    s_status mcu_status;

    /* check input parameters */
    CHECK_NULL(com_target);
    CHECK_NULL(temperature);

    usb_device = *(int *)com_target;

    if (mcu_get_status(usb_device, &mcu_status) != 0) {
        ERROR_PRINTF("ERROR: failed to getUplink status from the concentrator MCU\n");
        return -1;
    }
    INFO_PRINTF("Temperature:%.1foC\n", mcu_status.temperature);

    *temperature = mcu_status.temperature;

    return 0;
}

/* --- EOF ------------------------------------------------------------------ */
