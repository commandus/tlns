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
    lorawan/helper/ip-address.cpp lorawan/helper/thread-helper.cpp \
    lorawan/proto/gw/gw.cpp lorawan/proto/gw/set-gateway-metadata.cpp  lorawan/proto/gw/proto-gw-parser.cpp lorawan/proto/gw/basic-udp.cpp \
    lorawan/storage/gateway-identity.cpp lorawan/storage/network-identity.cpp \
    lorawan/task/message-queue-item.cpp lorawan/task/message-queue.cpp lorawan/task/task-descriptor.cpp \
    lorawan/task/message-task-dispatcher.cpp lorawan/task/task-response.cpp \
    lorawan/task/task-socket.cpp lorawan/task/task-udp-socket.cpp lorawan/task/task-udp-control-socket.cpp \
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
    lorawan/helper/file-helper.h \
    lorawan/helper/thread-helper.h \
    lorawan/helper/ip-address.h \
    lorawan/lorawan-conv.h \
    lorawan/lorawan-mac.h \
    lorawan/lorawan-const.h \
    lorawan/lorawan-error.h \
    lorawan/lorawan-date.h \
    lorawan/lorawan-msg.h \
    lorawan/lorawan-types.h \
    lorawan/task/message-queue.h            lorawan/task/task-accepted-socket.h  lorawan/task/task-response.h            lorawan/task/task-udp-socket.h \
    lorawan/task/message-queue-item.h       lorawan/task/task-descriptor.h       lorawan/task/task-socket.h              lorawan/task/task-unix-control-socket.h \
    lorawan/task/message-task-dispatcher.h  lorawan/task/task-platform.h         lorawan/task/task-udp-control-socket.h  lorawan/task/task-unix-socket.h \
    lorawan/lorawan-string.h

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
bin_PROGRAMS = cli-main-check

if ENABLE_GW_DEV_USB
bin_PROGRAMS += gw-dev-usb
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
	autogen.sh CMakeLists.txt CODE_OF_CONDUCT.md CONTRIBUTING.md COPYING HISTORY LICENSE README.md TODO \
	gw-dev/usb/CMakeLists.txt \
	third-party/argtable3/README.argtable3 \
	third-party/base64/README.base64 \
	third-party/libloragw/README.libloragw.md third-party/libloragw/LICENSE.libloragw.txt \
	third-party/libloragw/cp-lib.sh third-party/libloragw/build.sh \
	third-party/libloragw/CMakeLists.txt \
	third-party/nlohmann/README.nlohmann.md
