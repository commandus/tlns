SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4

EXTRA_LIB =
EXTRA_HEADERS =
EXTRA_DEF =

#
# https://ask-dev.ru/info/1247790/understanding-gcc-5s-glibcxxusecxx11abi-or-the-new-abi
#-D_GLIBCXX_USE_CXX11_ABI=0
# -I../.third-party for make distcheck target be happy
#
COMMON_C_FLAGS = -Ithird-party -I../third-party
AM_CXXFLAGS= -std=c++11 $(COMMON_C_FLAGS)
AM_CPPFLAGS = $(COMMON_C_FLAGS)

SRC_ARGTABLE = third-party/argtable3/argtable3.c
SRC_AES = third-party/system/crypto/aes.c third-party/system/crypto/cmac.c
SRC_LIBLORAWAN = \
    lorawan/lorawan-conv.cpp lorawan/lorawan-date.cpp lorawan/lorawan-error.cpp lorawan/lorawan-mac.cpp \
    lorawan/lorawan-msg.cpp lorawan/lorawan-string.cpp lorawan/lorawan-types.cpp lorawan/lorawan-packet-storage.cpp \
    lorawan/lorawan-builder.cpp lorawan/task/lorawan-mic.cpp lorawan/lorawan-key.cpp lorawan/helper/key128gen.cpp \
    lorawan/helper/aes-helper.cpp \
    lorawan/helper/ip-address.cpp lorawan/helper/thread-helper.cpp \
    lorawan/proto/gw/gw.cpp lorawan/proto/gw/set-gateway-metadata.cpp  lorawan/proto/gw/proto-gw-parser.cpp \
    lorawan/proto/gw/basic-udp.cpp \
    lorawan/regional-parameters/regional-parameter-channel-plan.cpp \
    lorawan/regional-parameters/regional-parameter-channel-plans.cpp \
    lorawan/regional-parameters/regional-parameter-channel-plan-file-json.cpp \
    lorawan/regional-parameters/regional-parameter-channel-plan-mem.cpp \
    lorawan/storage/gateway-identity.cpp lorawan/storage/network-identity.cpp \
    lorawan/task/message-queue-item.cpp lorawan/task/message-queue.cpp lorawan/task/task-descriptor.cpp \
    lorawan/task/message-task-dispatcher.cpp lorawan/task/task-response.cpp \
    lorawan/task/task-socket.cpp lorawan/task/task-udp-socket.cpp lorawan/task/task-udp-control-socket.cpp \
    lorawan/task/task-eventfd-control-socket.cpp lorawan/task/task-timer-socket.cpp lorawan/task/task-time-addr.cpp \
    lorawan/task/task-accepted-socket.cpp lorawan/task/task-unix-socket.cpp lorawan/task/task-unix-control-socket.cpp \
    third-party/strptime.cpp \
    third-party/base64/base64.cpp \
    $(SRC_AES)

LIBLORAGW_ROOT = third-party/libloragw
LIBLORAGW_SRC_DIR = $(LIBLORAGW_ROOT)/src
LIBLORAGW_INC_DIR = $(LIBLORAGW_ROOT)/inc
SRC_LIBLORAGW = \
    $(LIBLORAGW_SRC_DIR)/loragw_spi.c \
    $(LIBLORAGW_SRC_DIR)/loragw_usb.c \
    $(LIBLORAGW_SRC_DIR)/loragw_com.c \
    $(LIBLORAGW_SRC_DIR)/loragw_mcu.c \
    $(LIBLORAGW_SRC_DIR)/loragw_i2c.c \
    $(LIBLORAGW_SRC_DIR)/sx125x_spi.c \
    $(LIBLORAGW_SRC_DIR)/sx125x_com.c \
    $(LIBLORAGW_SRC_DIR)/sx1250_spi.c \
    $(LIBLORAGW_SRC_DIR)/sx1250_usb.c \
    $(LIBLORAGW_SRC_DIR)/sx1250_com.c \
    $(LIBLORAGW_SRC_DIR)/sx1261_spi.c \
    $(LIBLORAGW_SRC_DIR)/sx1261_usb.c \
    $(LIBLORAGW_SRC_DIR)/sx1261_com.c \
    $(LIBLORAGW_SRC_DIR)/loragw_aux.c \
    $(LIBLORAGW_SRC_DIR)/loragw_reg.c \
    $(LIBLORAGW_SRC_DIR)/loragw_sx1250.c \
    $(LIBLORAGW_SRC_DIR)/loragw_sx1261.c \
    $(LIBLORAGW_SRC_DIR)/loragw_sx125x.c \
    $(LIBLORAGW_SRC_DIR)/loragw_sx1302.c \
    $(LIBLORAGW_SRC_DIR)/loragw_cal.c \
    $(LIBLORAGW_SRC_DIR)/loragw_debug.c \
    $(LIBLORAGW_SRC_DIR)/loragw_hal.c \
    $(LIBLORAGW_SRC_DIR)/loragw_lbt.c \
    $(LIBLORAGW_SRC_DIR)/loragw_stts751.c \
    $(LIBLORAGW_SRC_DIR)/loragw_gps.c \
    $(LIBLORAGW_SRC_DIR)/loragw_sx1302_timestamp.c \
    $(LIBLORAGW_SRC_DIR)/loragw_sx1302_rx.c \
    $(LIBLORAGW_SRC_DIR)/loragw_ad5338r.c \
    $(LIBLORAGW_ROOT)/tinymt32.c \
    $(LIBLORAGW_ROOT)/jitqueue.c

nobase_dist_include_HEADERS = \
    task-response-threaded.h \
    third-party/argtable3/argtable3.h \
    third-party/base64/base64.h \
    third-party/daemonize.h \
    third-party/libloragw/jitqueue.h \
    third-party/libloragw/tinymt32.h \
    third-party/libloragw/gateway-lora.h \
    third-party/libloragw/inc/loragw_i2c.h \
    third-party/libloragw/inc/loragw_mcu.h \
    third-party/libloragw/inc/loragw_spi.h \
    third-party/libloragw/inc/sx125x_com.h \
    third-party/libloragw/inc/loragw_aux.h \
    third-party/libloragw/inc/sx1261_com.h \
    third-party/libloragw/inc/sx1250_com.h \
    third-party/libloragw/inc/loragw_stts751.h \
    third-party/libloragw/inc/loragw_usb.h \
    third-party/libloragw/inc/loragw_ad5338r.h \
    third-party/libloragw/inc/loragw_gps.h \
    third-party/libloragw/inc/loragw_sx1250.h \
    third-party/libloragw/inc/loragw_hal.h \
    third-party/libloragw/inc/loragw_sx125x.h \
    third-party/libloragw/inc/sx1250_spi.h \
    third-party/libloragw/inc/loragw_debug.h \
    third-party/libloragw/inc/loragw_sx1302.h \
    third-party/libloragw/inc/loragw_sx1302_timestamp.h \
    third-party/libloragw/inc/sx1250_usb.h \
    third-party/libloragw/inc/sx1250_defs.h \
    third-party/libloragw/inc/loragw_sx1302_rx.h \
    third-party/libloragw/inc/loragw_com.h \
    third-party/libloragw/inc/loragw_agc_params.h \
    third-party/libloragw/inc/sx1261_defs.h \
    third-party/libloragw/inc/sx1261_spi.h \
    third-party/libloragw/inc/loragw_lbt.h \
    third-party/libloragw/inc/loragw_sx1261.h \
    third-party/libloragw/inc/config.h \
    third-party/libloragw/inc/sx1261_usb.h \
    third-party/libloragw/inc/loragw_reg.h \
    third-party/libloragw/inc/sx125x_spi.h \
    third-party/libloragw/inc/loragw_cal.h \
    third-party/libloragw/subst-call-c.h \
    third-party/libloragw/trace.h \
    third-party/libloragw/subst.h \
    third-party/libloragw/gateway-settings.h \
    third-party/libloragw/libloragw-helper.h \
    third-party/nlohmann/json.hpp \
    third-party/strptime.h \
	third-party/system/crypto/cmac.h \
    third-party/system/crypto/aes.h \
    main/wifi-station.h \
    main/platform-defs.h \
    gw-dev/usb/rak2287.h \
    gw-dev/usb/task-usb-socket.h \
    gw-dev/usb/log-intf.h \
    lorawan/storage/network-identity.h \
    lorawan/storage/gateway-identity.h \
    lorawan/lorawan-packet-storage.h \
    lorawan/proto/gw/gw.h lorawan/proto/gw/set-gateway-metadata.h \
    lorawan/proto/gw/proto-gw-parser.h \
    lorawan/proto/gw/basic-udp.h \
    lorawan/proto/gw/gateway-file-json.h \
    lorawan/regional-parameters/regional-parameter-channel-plan-file-json.h \
    lorawan/regional-parameters/regional-parameter-channel-plan.h \
    lorawan/regional-parameters/regional-parameter-channel-plans.h \
    lorawan/regional-parameters/regional-parameter-channel-plan-file-json.h \
    lorawan/regional-parameters/regional-parameter-channel-plan-mem.h \
    lorawan/helper/file-helper.h lorawan/helper/thread-helper.h lorawan/helper/ip-address.h \
    lorawan/lorawan-conv.h lorawan/lorawan-mac.h lorawan/lorawan-const.h lorawan/lorawan-error.h \
    lorawan/lorawan-date.h lorawan/lorawan-msg.h lorawan/lorawan-types.h lorawan/task/lorawan-mic.h \
    lorawan/lorawan-key.h lorawan/helper/key128gen.h lorawan/helper/aes-helper.h \
    lorawan/task/message-queue.h lorawan/task/task-accepted-socket.h lorawan/task/task-response.h \
    lorawan/task/task-udp-socket.h lorawan/task/message-queue-item.h lorawan/task/task-descriptor.h \
    lorawan/task/task-socket.h lorawan/task/task-unix-control-socket.h lorawan/task/message-task-dispatcher.h \
    lorawan/task/task-platform.h lorawan/task/task-udp-control-socket.h  lorawan/task/task-unix-socket.h \
    lorawan/task/task-eventfd-control-socket.h lorawan/task/task-timer-socket.h lorawan/task/task-time-addr.h \
    lorawan/lorawan-string.h lorawan/lorawan-builder.h

#
# Libraries
#
lib_LIBRARIES = liblorawan.a
if ENABLE_GW_DEV_USB
lib_LIBRARIES += libloragw.a
endif

#
# Binaries
#
bin_PROGRAMS = cli-main-check regional-parameters2cpp

if ENABLE_GW_DEV_USB
bin_PROGRAMS += gw-dev-usb gateway-config2cpp 
endif

# Static library used for build binaries, so build library first.
$(bin_PROGRAMS): $(lib_LIBRARIES)

liblorawan_a_SOURCES = $(SRC_LIBLORAWAN)
liblorawan_a_CPPFLAGS = -fPIC

libloragw_a_SOURCES = $(SRC_LIBLORAGW)
libloragw_a_CPPFLAGS = -fPIC -I$(LIBLORAGW_ROOT) -I$(LIBLORAGW_INC_DIR)

cli_main_check_SOURCES = \
	cli-main-check.cpp task-response-threaded.cpp \
	$(SRC_ARGTABLE)
cli_main_check_LDADD = -L. -llorawan
cli_main_check_CPPFLAGS = $(EXTRA_DEF)

# -----------------------
gw_dev_usb_SOURCES = \
	gw-dev/usb/gw-dev-usb.cpp gw-dev/usb/task-usb-socket.cpp gw-dev/usb/rak2286.cpp \
    task-response-threaded.cpp \
    lorawan/helper/file-helper.cpp lorawan/helper/thread-helper.cpp lorawan/task/task-accepted-socket.cpp \
    third-party/daemonize.cpp third-party/libloragw/libloragw-helper.cpp third-party/libloragw/subst-call-c.cpp \
	$(SRC_ARGTABLE)
gw_dev_usb_LDADD = -L. -llorawan -lloragw
gw_dev_usb_CPPFLAGS = $(EXTRA_DEF) -Igw-dev/usb -Ithird-party/libloragw -Ithird-party/libloragw/inc


gateway_config2cpp_SOURCES = tools/gateway-config2cpp.cpp lorawan/proto/gw/gateway-file-json.cpp \
    $(SRC_ARGTABLE)
gateway_config2cpp_LDADD = -L. -llorawan -lloragw
gateway_config2cpp_CPPFLAGS = $(EXTRA_DEF) -Igw-dev/usb -Ithird-party/libloragw -Ithird-party/libloragw/inc

regional_parameters2cpp_SOURCES = tools/regional-parameters2cpp.cpp \
    $(SRC_ARGTABLE)
regional_parameters2cpp_LDADD = -L. -llorawan
regional_parameters2cpp_CPPFLAGS = $(EXTRA_DEF)

test_printf_SOURCES = tests/test-printf.cpp \
    third-party/libloragw/subst-call-c.cpp third-party/libloragw/libloragw-helper.cpp
test_printf_CPPFLAGS = -Ithird-party/libloragw -Igw-dev/usb

#
# test-printf
#
check_PROGRAMS = test-printf

TESTS = $(check_PROGRAMS)

#
# Configs, readme, CMake etc.
#
configdir = $(datadir)
dist_config_DATA = \
    gen/gateway-usb-conf.h \
    gen/regional-parameters.json \
    gen/regional-parameters-3.h \
	autogen.sh CMakeLists.txt CODE_OF_CONDUCT.md CONTRIBUTING.md COPYING HISTORY LICENSE README.md TODO \
	gw-dev/usb/CMakeLists.txt \
	third-party/argtable3/README.argtable3 \
	third-party/base64/README.base64 \
	third-party/libloragw/README.libloragw.md third-party/libloragw/LICENSE.libloragw.txt \
	third-party/libloragw/cp-lib.sh third-party/libloragw/build.sh \
	third-party/libloragw/CMakeLists.txt \
    third-party/libloragw/src/sx1261_pram.var third-party/libloragw/src/agc_fw_sx1250.var \
    third-party/libloragw/src/agc_fw_sx1257.var third-party/libloragw/src/cal_fw.var third-party/libloragw/src/arb_fw.var \
	third-party/nlohmann/README.nlohmann.md
