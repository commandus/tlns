#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>

#ifdef __cplusplus
#define C_CALL extern "C"
extern "C" {
#else
#define C_CALL
#endif

#include <stdio.h>
#include <malloc.h>
#include "stdint.h"
// library configuration options (dynamically generated)
#include "config.h"

#include "nanosleep/nanosleep.h"

// termios.h
typedef unsigned tcflag_t; /*This is an unsigned integer type used to represent the various bit masks for terminal flags.*/
typedef unsigned cc_t; /*This is an unsigned integer type used to represent characters associated with various terminal control functions.*/
typedef unsigned speed_t; /*used for terminal baud rates*/
#define NCCS 11
typedef struct termios {
    tcflag_t c_iflag; //input modes
    tcflag_t c_oflag; //output modes
    tcflag_t c_cflag; //control modes
    tcflag_t c_lflag; //local modes
    cc_t c_cc[NCCS]; //special character
} termios;

#define DEBUG_CRT   1

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#if DEBUG_CRT == 1
    #define DEBUG_PRINTF(fmt, ...)    fprintf(stdout, fmt, __VA_ARGS__); fflush(stdout)
#else
    #define DEBUG_PRINTF(fmt, ...)
#endif

#define ERROR_PRINTF(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__); fflush(stderr)
#define INFO_PRINTF(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__); fflush(stdout)

#define CHECK_NULL(a)                if(a==NULL){return -1;}
#define CHECK_ERR(a)                    if(a==-1){return LGW_REG_ERROR;}

// ---------------------- SPI stub --------------------------
// sx125x_spi.h
#include "sx1250_defs.h"
int sx1250_spi_w(void *com_target, uint8_t spi_mux_target, sx1250_op_code_t op_code, uint8_t *data, uint16_t size);
int sx1250_spi_r(void *com_target, uint8_t spi_mux_target, sx1250_op_code_t op_code, uint8_t *data, uint16_t size);

// loragw_spi.h
#define LGW_SPI_SUCCESS     0

int lgw_spi_open(const char * com_path, void **com_target_ptr);
int lgw_spi_close(void *com_target);
int lgw_spi_w(void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t data);
int lgw_spi_r(void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t *data);
int lgw_spi_rmw(void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t offs, uint8_t leng, uint8_t data);
int lgw_spi_wb(void *com_target, uint8_t spi_mux_target, uint16_t address, const uint8_t *data, uint16_t size);
int lgw_spi_rb(void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t *data, uint16_t size);
uint16_t lgw_spi_chunk_size(void);

// sx1261_spi.h
#include "sx1261_defs.h"
int sx1261_spi_w(void *com_target, sx1261_op_code_t op_code, uint8_t *data, uint16_t size);
int sx1261_spi_r(void *com_target, sx1261_op_code_t op_code, uint8_t *data, uint16_t size);

// sx125x_spi.h
int sx125x_spi_r(void *com_target, uint8_t spi_mux_target, uint8_t address, uint8_t *data);
int sx125x_spi_w(void *com_target, uint8_t spi_mux_target, uint8_t address, uint8_t data);

// loragw_i2c.h
#define LGW_I2C_SUCCESS     0
#define LGW_I2C_ERROR       -1
#define I2C_DEVICE          "/dev/i2c-1"
int i2c_linuxdev_open(const char *path, uint8_t device_addr, int *i2c_fd);
int i2c_linuxdev_close(int i2c_fd);
int i2c_linuxdev_read(int i2c_fd, uint8_t device_addr, uint8_t reg_addr, uint8_t *data);
int i2c_linuxdev_write(int i2c_fd, uint8_t device_addr, uint8_t reg_addr, uint8_t data);
int i2c_linuxdev_write_buffer(int i2c_fd, uint8_t device_addr, uint8_t *buffer, uint8_t size);
//
typedef int64_t ssize_t;
#define IXANY 0
#define O_NOCTTY 0
#define O_SYNC 0

//
#define CLOCK_MONOTONIC 0
#define clock_nanosleep(clock, flags, t, rem) nanosleep(t, rem)
//

// Serial
#define read readSerial
#define write writeSerial
#define close closeSerial
#define select selectSerial

int openSerial(const char* portName, int opt);
int closeSerial(int fd);
int readSerial(int fd, void* buffer, size_t count);
int writeSerial(int fd, const void* buffer, size_t count);
int setSerialSpeed(int fd, int baud);
int setSerialBlocking(int fd, BOOL blocking);

int gettimeofday(struct timeval* tp, struct timezone* tzp);

#ifdef __cplusplus
}
#endif
