#include "platform-win.h"
#include "win-fd-com.h"

// sx125x_spi.c

C_CALL int sx1250_spi_w(void *com_target, uint8_t spi_mux_target, sx1250_op_code_t op_code, uint8_t *data, uint16_t size)
{
    return 0;
}

C_CALL int sx1250_spi_r(void *com_target, uint8_t spi_mux_target, sx1250_op_code_t op_code, uint8_t *data, uint16_t size)
{
    return 0;
}

// loragw_spi.c

C_CALL int lgw_spi_open(const char * com_path, void **com_target_ptr)
{
    return 0;
}

C_CALL int lgw_spi_close(void *com_target)
{
    return 0;
}

C_CALL int lgw_spi_w(void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t data)
{
    return 0;
}

C_CALL int lgw_spi_r(void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t *data)
{
    return 0;
}

C_CALL int lgw_spi_rmw(void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t offs, uint8_t leng, uint8_t data)
{
    return 0;
}

C_CALL int lgw_spi_wb(void *com_target, uint8_t spi_mux_target, uint16_t address, const uint8_t *data, uint16_t size)
{
    return 0;
}

C_CALL int lgw_spi_rb(void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t *data, uint16_t size)
{
    return 0;
}

uint16_t lgw_spi_chunk_size(void)
{
    return 0;
}

// sx1261_spi.c

C_CALL int sx1261_spi_w(void *com_target, sx1261_op_code_t op_code, uint8_t *data, uint16_t size)
{
    return 0;
}

C_CALL int sx1261_spi_r(void *com_target, sx1261_op_code_t op_code, uint8_t *data, uint16_t size)
{
    return 0;
}

// sx125x_spi.c

C_CALL int sx125x_spi_r(void *com_target, uint8_t spi_mux_target, uint8_t address, uint8_t *data)
{
    return 0;
}

C_CALL int sx125x_spi_w(void *com_target, uint8_t spi_mux_target, uint8_t address, uint8_t data)
{
    return 0;
}

// loragw_i2c.c

C_CALL int i2c_linuxdev_open(const char *path, uint8_t device_addr, int *i2c_fd)
{
    return 0;
}

C_CALL int i2c_linuxdev_close(int i2c_fd)
{
    return 0;
}

C_CALL int i2c_linuxdev_read(int i2c_fd, uint8_t device_addr, uint8_t reg_addr, uint8_t *data)
{
    return 0;
}

C_CALL int i2c_linuxdev_write(int i2c_fd, uint8_t device_addr, uint8_t reg_addr, uint8_t data)
{
    return 0;
}

C_CALL int i2c_linuxdev_write_buffer(int i2c_fd, uint8_t device_addr, uint8_t *buffer, uint8_t size)
{
    return 0;
}

WinComPortCollection comPorts;

C_CALL int openSerial(const char* portName, int opt) {
    return comPorts.openPort(portName);
}

C_CALL int closeSerial(int fd) {
    return comPorts.closePort(fd);
}

C_CALL int readSerial(int fd, void* buffer, size_t count)
{
    return comPorts.readPort(fd, buffer, count);
}

C_CALL int writeSerial(int fd, const void* buffer, size_t count)
{
    return comPorts.writePort(fd, buffer, count);
}

C_CALL int setSerialSpeed(int fd, int baud)
{
    return comPorts.setSpeed(fd, baud);
}

C_CALL int setSerialBlocking(int fd, BOOL blocking)
{
    return comPorts.setBlocking(fd, blocking);
}

C_CALL int setSerialTimeout(int fd, int ms)
{
    return comPorts.setTimeout(fd, ms);
}
