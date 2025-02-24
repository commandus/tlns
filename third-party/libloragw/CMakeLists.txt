cmake_minimum_required(VERSION 3.9)
set(PROJECT_NAME "libloragw")
project(${PROJECT_NAME} VERSION 1.0)

set(LIBLORAGW_ROOT .)
file(REAL_PATH ${LIBLORAGW_ROOT} RAK_COMMON_FOR_GATEWAY_ROOT EXPAND_TILDE)

set(LIBLORAGW_SRC_DIR ${LIBLORAGW_ROOT}/src)
set(TINYMT32_INC_DIR ${LIBLORAGW_ROOT}/../libtools/inc)
set(LIBLORAGW_INC_DIR ${LIBLORAGW_ROOT}/inc)
set(JITQUEUE_INC_DIR ${LIBLORAGW_ROOT}/../packet_forwarder/inc)

set(LIBLORAGW_INC_DIRS . ${LIBLORAGW_ROOT} ${LIBLORAGW_INC_DIR} ${TINYMT32_INC_DIR} ${JITQUEUE_INC_DIR})

set(LIBLORAGW_SRC
    ${LIBLORAGW_SRC_DIR}/loragw_spi.c
    ${LIBLORAGW_SRC_DIR}/loragw_usb.c
    ${LIBLORAGW_SRC_DIR}/loragw_com.c
    ${LIBLORAGW_SRC_DIR}/loragw_mcu.c
    ${LIBLORAGW_SRC_DIR}/loragw_i2c.c
    ${LIBLORAGW_SRC_DIR}/sx125x_spi.c
    ${LIBLORAGW_SRC_DIR}/sx125x_com.c
    ${LIBLORAGW_SRC_DIR}/sx1250_spi.c
    ${LIBLORAGW_SRC_DIR}/sx1250_usb.c
    ${LIBLORAGW_SRC_DIR}/sx1250_com.c
    ${LIBLORAGW_SRC_DIR}/sx1261_spi.c
    ${LIBLORAGW_SRC_DIR}/sx1261_usb.c
    ${LIBLORAGW_SRC_DIR}/sx1261_com.c
    ${LIBLORAGW_SRC_DIR}/loragw_aux.c
    ${LIBLORAGW_SRC_DIR}/loragw_reg.c
    ${LIBLORAGW_SRC_DIR}/loragw_sx1250.c
    ${LIBLORAGW_SRC_DIR}/loragw_sx1261.c
    ${LIBLORAGW_SRC_DIR}/loragw_sx125x.c
    ${LIBLORAGW_SRC_DIR}/loragw_sx1302.c
    ${LIBLORAGW_SRC_DIR}/loragw_cal.c
    ${LIBLORAGW_SRC_DIR}/loragw_debug.c
    ${LIBLORAGW_SRC_DIR}/loragw_hal.c
    ${LIBLORAGW_SRC_DIR}/loragw_lbt.c
    ${LIBLORAGW_SRC_DIR}/loragw_stts751.c
    ${LIBLORAGW_SRC_DIR}/loragw_gps.c
    ${LIBLORAGW_SRC_DIR}/loragw_sx1302_timestamp.c
    ${LIBLORAGW_SRC_DIR}/loragw_sx1302_rx.c
    ${LIBLORAGW_SRC_DIR}/loragw_ad5338r.c
    #   Tiny Mersenne Twister only 127 bit internal state
    ${LIBLORAGW_ROOT}/tinymt32.c
    ${LIBLORAGW_ROOT}/jitqueue.c
)
#
#    ${LIBLORAGW_ROOT}/../packet_forwarder/src/jitqueue.c

add_library(loragw STATIC ${LIBLORAGW_SRC})
set_property(TARGET loragw PROPERTY C_STANDARD 99)
target_include_directories(loragw PRIVATE ${LIBLORAGW_INC_DIRS})
target_compile_options(loragw PRIVATE -include subst.h)
