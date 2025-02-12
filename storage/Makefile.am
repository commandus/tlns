SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4
VERSION = 1.0.0
VERSION_INFO = 1:0:0

# ../../third-party for make distcheck happy
common_flags = -I. -Ithird-party -I../../third-party

AM_CPPFLAGS=$(common_flags)
AM_CXXFLAGS=$(common_flags) -std=c++11
# COMMON_CPP_FLAGS = -D_GLIBCXX_USE_CXX11_ABI=0

ARGTABLE_SRC = third-party/argtable3/argtable3.c
AES_SRC = third-party/system/crypto/aes.c third-party/system/crypto/cmac.c

GATEWAY_DEF = $(COMMON_CPP_FLAGS)

# Static library used for build binaries, so build library first.
$(bin_PROGRAMS): $(lib_LIBRARIES)

lib_LIBRARIES = liblorawan.a

lib_LTLIBRARIES = libstorage-mem.la libstorage-gen.la

if ENABLE_JSON
lib_LTLIBRARIES += libstorage-json.la
endif
if ENABLE_SQLITE
lib_LTLIBRARIES += libstorage-sqlite.la
endif

#
# Binaries
#
bin_PROGRAMS = lorawan-service lorawan-query lorawan-query-direct lorawan-tag

SRC_ARGTABLE = third-party/argtable3/argtable3.c

nobase_dist_include_HEADERS = \
    lorawan/helper/aes-const.h \
    lorawan/helper/aes-helper.h \
    lorawan/helper/crc-helper.h \
    lorawan/helper/file-helper.h \
    lorawan/helper/ip-address.h \
    lorawan/helper/ip-helper.h \
    lorawan/helper/key128gen.h \
    lorawan/helper/sqlite-helper.h \
    lorawan//helper/uv-mem.h \
    lorawan/lorawan-const.h \
    lorawan/lorawan-conv.h \
    lorawan/lorawan-date.h \
    lorawan/lorawan-error.h \
    lorawan/lorawan-key.h \
    lorawan/lorawan-mac.h \
    lorawan/lorawan-mic.h \
    lorawan/lorawan-msg.h \
    lorawan/lorawan-packet-storage.h \
    lorawan/lorawan-string.h \
    lorawan/lorawan-types.h \
    lorawan/storage/service/identity-service-udp.h \
    lorawan/storage/client/direct-client.h \
    lorawan/storage/client/plugin-client.h \
    lorawan/storage/client/plugin-query-client.h \
    lorawan/storage/client/query-client.h \
    lorawan/storage/client/response-client.h \
    lorawan/storage/client/service-client.h \
    lorawan/storage/client/sync-query-client.h \
    lorawan/storage/client/sync-response-client.h \
    lorawan/storage/client/udp-client.h \
    lorawan/storage/client/uv-client.h \
    lorawan/storage/gateway-identity.h \
    lorawan/storage/listener/http-listener.h \
    lorawan/storage/listener/storage-listener.h \
    lorawan/storage/listener/udp-listener.h \
    lorawan/storage/listener/uv-listener.h \
    lorawan/storage/network-identity.h \
    lorawan/storage/serialization/gateway-binary-serialization.h \
    lorawan/storage/serialization/gateway-serialization.h \
    lorawan/storage/serialization/gateway-text-json-serialization.h \
    lorawan/storage/serialization/identity-binary-serialization.h \
    lorawan/storage/serialization/identity-serialization.h \
    lorawan/storage/serialization/identity-text-json-serialization.h \
    lorawan/storage/serialization/identity-text-urn-serialization.h \
    lorawan/storage/serialization/json-helper.h \
    lorawan/storage/serialization/qr-helper.h \
    lorawan/storage/serialization/serialization.h \
    lorawan/storage/serialization/service-serialization.h \
    lorawan/storage/serialization/urn-helper.h \
    lorawan/storage/service/async-wrapper-gateway-service.h \
    lorawan/storage/service/async-wrapper-identity-service.h \
    lorawan/storage/service/gateway-service.h \
    lorawan/storage/service/gateway-service-json.h \
    lorawan/storage/service/gateway-service-mem.h \
    lorawan/storage/service/gateway-service-sqlite.h \
    lorawan/storage/service/identity-service-gen.h \
    lorawan/storage/service/identity-service.h \
    lorawan/storage/service/identity-service-json.h \
    lorawan/storage/service/identity-service-mem.h \
    lorawan/storage/service/identity-service-sqlite.h \
    lorawan/task/task-platform.h \
    third-party/argtable3/argtable3.h \
    third-party/daemonize.h \
    third-party/nayuki/qrcodegen.hpp \
    third-party/nlohmann/json.hpp \
    third-party/base64/base64.h \
    third-party/strptime.h \
    third-party/system/crypto/aes.h \
    third-party/system/crypto/cmac.h \
    cli-helper.h \
    config.h \
    log.h \
    lorawan/helper/plugin-helper.h

#
# liblorawan
#
SRC_LIBLORAWAN = \
    lorawan/helper/aes-helper.cpp \
    lorawan/helper/crc-helper.cpp \
    lorawan/helper/file-helper.cpp \
    lorawan/helper/ip-address.cpp \
    lorawan/helper/ip-helper.cpp \
    lorawan/helper/key128gen.cpp \
    lorawan/helper/sqlite-helper.cpp \
    lorawan/lorawan-conv.cpp \
    lorawan/lorawan-date.cpp \
    lorawan/lorawan-error.cpp \
    lorawan/lorawan-key.cpp \
    lorawan/lorawan-mic.cpp \
    lorawan/lorawan-mac.cpp \
    lorawan/lorawan-msg.cpp \
    lorawan/lorawan-packet-storage.cpp \
    lorawan/lorawan-string.cpp \
    lorawan/lorawan-types.cpp \
    lorawan/storage/service/identity-service-udp.cpp \
    lorawan/storage/client/direct-client.cpp \
    lorawan/storage/client/plugin-client.cpp \
    lorawan/storage/client/plugin-query-client.cpp \
    lorawan/storage/client/query-client.cpp \
    lorawan/storage/client/service-client.cpp \
    lorawan/storage/client/sync-query-client.cpp \
    lorawan/storage/client/sync-response-client.cpp \
    lorawan/storage/client/udp-client.cpp \
    lorawan/storage/gateway-identity.cpp \
    lorawan/storage/listener/storage-listener.cpp \
    lorawan/storage/listener/udp-listener.cpp \
    lorawan/storage/network-identity.cpp \
    lorawan/storage/serialization/gateway-binary-serialization.cpp \
    lorawan/storage/serialization/gateway-serialization.cpp \
	lorawan/storage/serialization/identity-binary-serialization.cpp \
    lorawan/storage/serialization/identity-serialization.cpp \
    lorawan/storage/serialization/identity-text-urn-serialization.cpp \
    lorawan/storage/serialization/serialization.cpp \
    lorawan/storage/serialization/service-serialization.cpp \
    lorawan/storage/serialization/urn-helper.cpp \
    lorawan/storage/service/async-wrapper-gateway-service.cpp \
    lorawan/storage/service/async-wrapper-identity-service.cpp \
    lorawan/storage/service/gateway-service.cpp \
    lorawan/storage/service/gateway-service-json.cpp \
    lorawan/storage/service/gateway-service-mem.cpp \
    lorawan/storage/service/identity-service.cpp \
    lorawan/storage/service/identity-service-gen.cpp \
    lorawan/storage/service/identity-service-json.cpp \
    lorawan/storage/service/identity-service-mem.cpp \
    third-party/base64/base64.cpp \
    third-party/strptime.cpp \
    ${AES_SRC}

EXTRA_LIB =

if ENABLE_LIBUV
SRC_LIBLORAWAN += lorawan/helper/uv-mem.cpp lorawan/storage/client/uv-client.cpp lorawan/storage/listener/uv-listener.cpp
GATEWAY_DEF += -DENABLE_LIBUV
EXTRA_LIB += -luv
endif

if ENABLE_GEN
GATEWAY_DEF += -DENABLE_GEN
endif

if ENABLE_JSON
GATEWAY_DEF += -DENABLE_JSON
SRC_LIBLORAWAN += \
    lorawan/storage/serialization/gateway-text-json-serialization.cpp \
    lorawan/storage/serialization/identity-text-json-serialization.cpp \
    lorawan/storage/serialization/json-helper.cpp
endif

if ENABLE_SQLITE
GATEWAY_DEF += -DENABLE_SQLITE
EXTRA_LIB += -lsqlite3
SRC_LIBLORAWAN += \
    lorawan/storage/service/gateway-service-sqlite.cpp \
    lorawan/storage/service/identity-service-sqlite.cpp
endif

if ENABLE_HTTP
GATEWAY_DEF += -DENABLE_HTTP
EXTRA_LIB += -lmicrohttpd
SRC_LIBLORAWAN += \
    lorawan/storage/listener/http-listener.cpp
endif

if ENABLE_QRCODE
GATEWAY_DEF += -DENABLE_QRCODE
SRC_LIBLORAWAN += \
    lorawan/storage/serialization/qr-helper.cpp
endif

liblorawan_a_SOURCES = $(SRC_LIBLORAWAN)
liblorawan_a_CPPFLAGS = -fPIC

lorawan_query_SOURCES = \
    cli-query-main.cpp cli-helper.cpp \
    lorawan/storage/network-identity.cpp lorawan/helper/aes-helper.cpp \
	$(SRC_ARGTABLE)
lorawan_query_LDADD = -L. -llorawan $(EXTRA_LIB)
lorawan_query_CPPFLAGS = $(GATEWAY_DEF)

lorawan_service_SOURCES = \
    cli-main.cpp \
    cli-helper.cpp \
    third-party/daemonize.cpp \
    lorawan/storage/network-identity.cpp lorawan/helper/aes-helper.cpp \
    $(SRC_ARGTABLE)
lorawan_service_LDADD = -L. -llorawan $(EXTRA_LIB)
lorawan_service_CPPFLAGS = $(GATEWAY_DEF) -DLNS_VERSION=$(VERSION)

lorawan_query_direct_SOURCES = \
    cli-query-plugin-main.cpp \
    cli-helper.cpp \
    lorawan/storage/network-identity.cpp lorawan/helper/aes-helper.cpp \
    $(SRC_ARGTABLE)
lorawan_query_direct_LDADD = -L. -llorawan $(EXTRA_LIB) 
lorawan_query_direct_CPPFLAGS = $(GATEWAY_DEF)

lorawan_tag_SOURCES = \
    cli-tag.cpp \
    cli-helper.cpp \
    third-party/nayuki/qrcodegen.cpp \
    $(SRC_ARGTABLE)
lorawan_tag_LDADD = -L. -llorawan $(EXTRA_LIB)
lorawan_tag_CPPFLAGS = $(GATEWAY_DEF)

#
# Plugins
#
libstorage_mem_la_SOURCES = \
    lorawan/storage/service/identity-service-mem.cpp \
    lorawan/storage/service/gateway-service-mem.cpp
libstorage_mem_la_LIBADD = -L. -llorawan
#libstorage_mem_la_LDFLAGS = -version-info $(VERSION_INFO)

libstorage_json_la_SOURCES = \
    lorawan/storage/service/identity-service-json.cpp \
    lorawan/storage/service/gateway-service-json.cpp \
	$(AES_SRC)
libstorage_json_la_LIBADD = -L. -llorawan
libstorage_json_la_CPPFLAGS = -Ithird-party
#libstorage_json_la_LDFLAGS = -version-info $(VERSION_INFO)

libstorage_gen_la_SOURCES = \
    lorawan/storage/service/identity-service-gen.cpp \
    lorawan/storage/service/gateway-service-mem.cpp \
	lorawan/helper/key128gen.cpp $(AES_SRC)
libstorage_gen_la_LIBADD = -L. -llorawan
libstorage_gen_la_CPPFLAGS = -Ithird-party
#libstorage_gen_la_LDFLAGS = -version-info $(VERSION_INFO)

libstorage_sqlite_la_SOURCES = \
    lorawan/storage/service/identity-service-sqlite.cpp \
    lorawan/storage/service/gateway-service-sqlite.cpp \
	lorawan/helper/file-helper.cpp \
	lorawan/helper/sqlite-helper.cpp
libstorage_sqlite_la_LIBADD = -L. -llorawan $(EXTRA_LIB)

#
# Configs, readme, CMake etc.
#
configdir = $(datadir)
dist_config_DATA = \
	autogen.sh CMakeLists.txt CODE_OF_CONDUCT.md CONTRIBUTING.md COPYING HISTORY LICENSE README.md TODO \
	tests/test-parse-packet.cpp \
	examples/example-direct.cpp examples/example-gen.cpp examples/example-gw-mem.cpp examples/example-mem.cpp \
	third-party/argtable3/README \
	third-party/base64/README.base64 \
    third-party/nayuki/README.nayuki \
	third-party/nlohmann/README.nlohmann.md

#
# Tests
#

# test_key_SOURCES = tests/test-parse-packet.cpp
#check_PROGRAMS =
#if ENABLE_TESTS
#	check_PROGRAMS += test-parse-packet
#endif
#
#TESTS = $(check_PROGRAMS)
