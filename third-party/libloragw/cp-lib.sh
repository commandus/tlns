#!/bin/sh
RAK_COMMON_FOR_GATEWAY_ROOT=~/src/rak_common_for_gateway
#
# ----- DO NOT EDIT BELOW THIS LINE -----
#
SRC=./src/
INC=./inc/
LIBLORAGW_ROOT=$RAK_COMMON_FOR_GATEWAY_ROOT/lora/rak2287/sx1302_hal/libloragw
LIBLORAGW_SRC_DIR=$LIBLORAGW_ROOT/src
LIBLORAGW_INC_DIR=$LIBLORAGW_ROOT/inc
cp $LIBLORAGW_SRC_DIR/loragw_spi.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_usb.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_com.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_mcu.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_i2c.c $SRC
cp $LIBLORAGW_SRC_DIR/sx125x_spi.c $SRC
cp $LIBLORAGW_SRC_DIR/sx125x_com.c $SRC
cp $LIBLORAGW_SRC_DIR/sx1250_spi.c $SRC
cp $LIBLORAGW_SRC_DIR/sx1250_usb.c $SRC
cp $LIBLORAGW_SRC_DIR/sx1250_com.c $SRC
cp $LIBLORAGW_SRC_DIR/sx1261_spi.c $SRC
cp $LIBLORAGW_SRC_DIR/sx1261_usb.c $SRC
cp $LIBLORAGW_SRC_DIR/sx1261_com.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_aux.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_reg.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_sx1250.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_sx1261.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_sx125x.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_sx1302.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_cal.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_debug.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_hal.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_lbt.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_stts751.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_gps.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_sx1302_timestamp.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_sx1302_rx.c $SRC
cp $LIBLORAGW_SRC_DIR/loragw_ad5338r.c $SRC
cp $LIBLORAGW_SRC_DIR/sx1261_pram.var $SRC
cp $LIBLORAGW_SRC_DIR/cal_fw.var $SRC
cp $LIBLORAGW_SRC_DIR/arb_fw.var $SRC
cp $LIBLORAGW_SRC_DIR/agc_fw_sx1250.var $SRC
cp $LIBLORAGW_SRC_DIR/agc_fw_sx1257.var $SRC

cp $LIBLORAGW_INC_DIR/* $INC

exit 0

