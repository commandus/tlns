#
# On Windows set correct path to the VCPKG includes at line 28
# mkdir build; cd build; cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\git\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DENABLE_SQLITE=on
#
# Options:
# -DENABLE_GW_DEV_USB=on    build RAK2287 USB gateway backend
# -DENABLE_LIBUV=off   		enable libuv UDP/TCP. Default UDP only. libuv dependency required.
# -DENABLE_DEBUG=off   		enable debugging output
# -DENABLE_GEN=on   		enable key generator (default in memory storage)
# -DENABLE_SQLITE=off  		enable SQLite3 backend. sqlite3 libsqlite3-dev dependencies required.
# -DENABLE_LMDB=off  		enable LMDB backend. liblmdb++-dev (and liblmdb-dev) dependencies required.
# -DENABLE_JSON=off   	    enable JSON file backend
# -DENABLE_MQTT=off			build with MQTT bridge
# -DENABLE_IPV6=off   		enable IPv6 (reserved)
#  storage specific options
# -DENABLE_HTTP=off			enable HTTP storage service. libmicrohttpd dependency required.
# -DENABLE_QRCODE=off		enable HTTP QRCode URN storage service.
# -DENABLE_MINIZIP=off		Build storage with minizip

# Example:
# cmake -DENABLE_LIBUV=on
#
cmake_minimum_required(VERSION 3.9)
set(VERSION_INFO 1.0.3)

if(${ESP_PLATFORM})
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
	project("tlns" VERSION ${VERSION_INFO})
#    get_filename_component(EXTRA_COMPONENT_DIRS esp32 ABSOLUTE)
#    set(IDF_EXTRA_COMPONENT_DIRS ${EXTRA_COMPONENT_DIRS})
else()
	project("tlns" VERSION ${VERSION_INFO})
    # ------------------------ CHANGE PATH BELOW ON WINDOWS ------------------------
    set(VCPKG_INC "/git/vcpkg/installed/x64-windows/include")
    # ------------------------ CHANGE PATH ABOVE ON WINDOWS ------------------------
	set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
	include_directories(${VCPKG_INC})

	#
	# Setting up i18l & l10n
	# see https://stackoverflow.com/questions/10521635/using-intltool-with-cmake
	#
	find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
	if(NOT GETTEXT_MSGFMT_EXECUTABLE)
		message("NOTE: msgfmt not found. Translations will *not* be installed")
	else()
		find_package(Intl)
		find_package(Gettext)
		if(Intl_LIBRARY)
			set(LIBINTL Intl::Intl)
		endif()
		include_directories(${Intl_INCLUDE_DIRS})
		link_directories(${Intl_LIBRARY_DIRS})
		# Build the .po files
		set(PO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/po)
		# Input files
		file(GLOB PO_FILES ${PO_PATH}/*.po)
		# Generated file list
		set(MO_FILES)
		set(PRG tlns)
		set(LOCALE_INSTALL_DIR /usr/share/locale)
		foreach (_poFile ${PO_FILES})
			get_filename_component(_poFileName ${_poFile} NAME)
			string(REGEX REPLACE "^${PRG}\\." "" _langCode ${_poFileName} )
			string(REGEX REPLACE "\\.UTF-8\\.po$" "" _langCode ${_langCode} )
			string(REGEX REPLACE "_..$" "" _langCode ${_langCode} )
			if ( _langCode )
				message("** PO file ${_poFileName} LANG ${_langCode} LIBDIR ${INTL_LIBRARY_DIRS} INCDIR ${INTL_INCLUDE_DIRS}")
				get_filename_component(_lang ${_poFile} NAME_WE)
	
				set(_moPath ${CMAKE_CURRENT_SOURCE_DIR}/locale/${_langCode}/LC_MESSAGES)
				set(_moFile ${_moPath}/${PRG}.mo)
				message("** ** ${_moFile} install to ${LOCALE_INSTALL_DIR}/${_langCode}/LC_MESSAGES/${PRG}.mo")
				#	mkdir -p ${_moPath} ; ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_moFile} ${_poFile}
				if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
					message("** powershell ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.ps1 ${GETTEXT_MSGFMT_EXECUTABLE} ${_moPath} ${_moFile} ${_poFile}")
					add_custom_command(OUTPUT ${_moFile}
						COMMAND powershell ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.ps1 ${GETTEXT_MSGFMT_EXECUTABLE} ${_moPath} ${_moFile} ${_poFile}
						DEPENDS ${_poFile})
				else()
					message("** ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.sh ${_moPath} ${_moFile} ${_poFile}")
					add_custom_command(OUTPUT ${_moFile}
						COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.sh ${_moPath} ${_moFile} ${_poFile}
						DEPENDS ${_poFile})
				endif()
	#			INSTALL(FILES ${_moFile} DESTINATION ${LOCALE_INSTALL_DIR}/${_langCode}/LC_MESSAGES/ RENAME ${PRG}.mo)
				list(APPEND MO_FILES ${_moFile})
			endif()
		endforeach (_poFile ${PO_FILES})
		add_custom_target(translations ALL DEPENDS ${MO_FILES})
	endif()

	set(CMAKE_CXX_STANDARD 11)

	option(ENABLE_GW_DEV_USB "Build RAK2287 USB gateway backend" ON)
	option(ENABLE_IPV6 "Build with IPv6" OFF)
	option(ENABLE_DEBUG "Build with debug" OFF)
	option(ENABLE_LIBUV "Build with libuv" OFF)
	option(ENABLE_GEN "Build with generator" OFF)
	option(ENABLE_SQLITE "Build with SQLite" OFF)
	option(ENABLE_LMDB "Build with LMDB" OFF)
	option(ENABLE_MQTT "Build with MQTT" OFF)
	option(ENABLE_JSON "Build with JSON file" ON)
	option(ENABLE_HTTP "Build storage with HTTP" OFF)
	option(ENABLE_QRCODE "Build storage with HTTP QRCode URN" OFF)
	option(ENABLE_MINIZIP "Build storage with minizip" OFF)

	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)

	set(ARGTABLE "third-party/argtable3/argtable3.c")
	set(AES_SRC third-party/system/crypto/aes.c third-party/system/crypto/cmac.c)

	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		# avoid Error LNK2038 mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
		set (OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
		add_subdirectory("third-party/libloragw-win")
	else ()
		add_subdirectory("third-party/libloragw")
	endif ()

	set(SRC_LIBLORAWAN
		lorawan/bridge/app-bridge.cpp
		lorawan/bridge/plugin-bridge.cpp
		lorawan/helper/aes-helper.cpp
		lorawan/helper/crc-helper.cpp
		lorawan/helper/file-helper.cpp
		lorawan/helper/ip-address.cpp
		lorawan/helper/ip-helper.cpp
		lorawan/helper/key128gen.cpp
		lorawan/helper/thread-helper.cpp
		lorawan/helper/tlns-cli-helper.cpp
		lorawan/lorawan-builder.cpp
		lorawan/lorawan-conv.cpp
		lorawan/lorawan-date.cpp
		lorawan/lorawan-error.cpp
		lorawan/lorawan-key.cpp
		lorawan/lorawan-mac.cpp
		lorawan/lorawan-mic.cpp
		lorawan/lorawan-msg.cpp
		lorawan/lorawan-packet-storage.cpp
		lorawan/lorawan-string.cpp
		lorawan/lorawan-types.cpp
		lorawan/power-dbm.cpp
		lorawan/proto/gw/basic-udp.cpp
		lorawan/proto/gw/gw.cpp
		lorawan/proto/gw/json-wired-client.cpp
		lorawan/proto/gw/json-wired.cpp
		lorawan/proto/gw/parse-result.cpp
		lorawan/proto/gw/proto-gw-parser.cpp
		lorawan/proto/gw/set-gateway-metadata.cpp
		lorawan/proto/payload2device/payload2device-parser.cpp
		lorawan/regional-parameters/regional-parameter-channel-plan-mem.cpp
		lorawan/regional-parameters/regional-parameter-channel-plan.cpp
		lorawan/regional-parameters/regional-parameter-channel-plans.cpp
		lorawan/storage/client/device-best-gateway-direct-client.cpp
		lorawan/storage/client/direct-client.cpp
		lorawan/storage/client/plugin-client.cpp
		lorawan/storage/client/plugin-query-client.cpp
		lorawan/storage/client/query-client.cpp
		lorawan/storage/client/service-client.cpp
		lorawan/storage/client/sync-query-client.cpp
		lorawan/storage/client/sync-response-client.cpp
		lorawan/storage/client/udp-client.cpp
		lorawan/storage/gateway-identity.cpp
		lorawan/storage/listener/storage-listener.cpp
		lorawan/storage/listener/udp-listener.cpp
		lorawan/storage/network-identity.cpp
		lorawan/storage/serialization/gateway-binary-serialization.cpp
		lorawan/storage/serialization/gateway-serialization.cpp
		lorawan/storage/serialization/identity-binary-serialization.cpp
		lorawan/storage/serialization/identity-serialization.cpp
		lorawan/storage/serialization/identity-text-urn-serialization.cpp
		lorawan/storage/serialization/serialization.cpp
		lorawan/storage/serialization/service-serialization.cpp
		lorawan/storage/serialization/urn-helper.cpp
		lorawan/storage/service/device-best-gateway-mem.cpp
		lorawan/storage/service/device-best-gateway.cpp
		lorawan/storage/service/gateway-service-mem.cpp
		lorawan/storage/service/gateway-service.cpp
		lorawan/storage/service/identity-service-c-wrapper.cpp
		lorawan/storage/service/identity-service-gen.cpp
		lorawan/storage/service/identity-service-json.cpp
		lorawan/storage/service/identity-service-mem.cpp
		lorawan/storage/service/identity-service-udp.cpp
		lorawan/storage/service/identity-service.cpp
		lorawan/task/message-queue-item.cpp
		lorawan/task/message-queue.cpp
		lorawan/task/message-task-dispatcher.cpp
		lorawan/task/task-accepted-socket.cpp
		lorawan/task/task-descriptor.cpp
		lorawan/task/task-response.cpp
		lorawan/task/task-socket.cpp
		lorawan/task/task-time-addr.cpp
		lorawan/task/task-timer-socket.cpp
		lorawan/task/task-udp-control-socket.cpp
		lorawan/task/task-udp-socket.cpp
		third-party/base64/base64.cpp
		third-party/strptime.cpp
		${AES_SRC}
	)

	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	else ()
		set(SRC_LIBLORAWAN ${SRC_LIBLORAWAN}
			lorawan/task/task-unix-socket.cpp
			lorawan/task/task-unix-control-socket.cpp
			lorawan/task/task-eventfd-control-socket.cpp
		)
	endif ()

	if (ENABLE_GW_DEV_USB)
		set(TLNS_DEF ${TLNS_DEF} ENABLE_GW_DEV_USB )
		add_subdirectory("gw-dev/usb")
	endif()

	if(ENABLE_HTTP)
		set(SRC_LIBLORAWAN ${SRC_LIBLORAWAN} lorawan/storage/listener/http-listener.cpp)
		set(TLNS_DEF ${TLNS_DEF} ENABLE_HTTP)
		find_library(LIBMICROHTTPD NAMES microhttpd libmicrohttpd-dll.lib HINTS /usr/lib/x86_64-linux-gnu/ ${VCPKG_LIB} ${MINGW_LIB})
	endif()

	if(ENABLE_QRCODE)
		set(TLNS_DEF ${TLNS_DEF} ENABLE_QRCODE)
		set(SRC_LIBLORAWAN ${SRC_LIBLORAWAN} lorawan/storage/serialization/qr-helper.cpp third-party/nayuki/qrcodegen.cpp)
	endif()

	if (ENABLE_IPV6)
		set(TLNS_DEF ${TLNS_DEF} ENABLE_IPV6 )
	endif()

	if (ENABLE_DEBUG)
		set(TLNS_DEF ${TLNS_DEF} ENABLE_DEBUG)
	endif()

	if (ENABLE_LIBUV)
		set(SRC_LIBLORAWAN ${SRC_LIBLORAWAN} lorawan/helper/uv-mem.cpp
				lorawan/storage/client/uv-client.cpp lorawan/storage/listener/uv-listener.cpp
		)
		set(TLNS_DEF ${TLNS_DEF} ENABLE_LIBUV)
		if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
			set(LIBUV LibUV)
			set(LIBUVA $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,uv>)
		else()
			set(LIBUV LibUV)
			set(LIBUVA LibUV::LibUV)
		endif()
		find_package(${LIBUV})
	endif()

	if (ENABLE_GEN)
		set(TLNS_DEF ${TLNS_DEF} ENABLE_GEN)
	endif()

	if (ENABLE_SQLITE)
		set(TLNS_DEF ${TLNS_DEF} ENABLE_SQLITE)
		set(SRC_LIBLORAWAN ${SRC_LIBLORAWAN} lorawan/storage/service/identity-service-sqlite.cpp lorawan/storage/service/gateway-service-sqlite.cpp
				lorawan/helper/sqlite-helper.cpp)
		if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
			find_package(unofficial-sqlite3 CONFIG REQUIRED)
			set(BACKEND_DB_LIB ${BACKEND_DB_LIB} unofficial::sqlite3::sqlite3)
			set(BACKEND_DB_INC ${BACKEND_DB_INC} ${SQLite3_INCLUDE_DIRS})
		else ()
			find_package(SQLite3 REQUIRED)
			set(BACKEND_DB_LIB ${BACKEND_DB_LIB} ${SQLite3_LIBRARIES})
			set(BACKEND_DB_INC ${BACKEND_DB_INC} ${SQLite3_INCLUDE_DIRS})
		endif ()
	endif()

	if (ENABLE_LMDB)
		set(TLNS_DEF ${TLNS_DEF} ENABLE_LMDB)
		set(SRC_LIBLORAWAN ${SRC_LIBLORAWAN} lorawan/storage/service/identity-service-lmdb.cpp lorawan/storage/service/gateway-service-lmdb.cpp lorawan/helper/lmdb-helper.cpp)
		if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
			find_package(unofficial-lmdb CONFIG REQUIRED)
			set(BACKEND_DB_LIB ${BACKEND_DB_LIB} unofficial::lmdb::lmdb)
			set(BACKEND_DB_INC ${BACKEND_DB_INC} ${LMDB_INCLUDE_DIRS})
		else ()
			find_library(LIBLMDB NAMES lmdb)
			set(BACKEND_DB_LIB ${BACKEND_DB_LIB} ${LIBLMDB})
		endif ()
	endif()

	if (ENABLE_JSON)
		set(TLNS_DEF ${TLNS_DEF} ENABLE_JSON)
		set(SRC_LIBLORAWAN ${SRC_LIBLORAWAN}
			lorawan/regional-parameters/regional-parameter-channel-plan-file-json.cpp
			lorawan/storage/serialization/json-helper.cpp
			lorawan/storage/serialization/identity-text-json-serialization.cpp
			lorawan/storage/serialization/gateway-text-json-serialization.cpp
			lorawan/helper/crc-helper.h
			lorawan/helper/crc-helper.cpp
		)
	endif()

	add_library(lorawan STATIC ${SRC_LIBLORAWAN})
	# -fPIC
	set_property(TARGET lorawan PROPERTY POSITION_INDEPENDENT_CODE ON)
	target_compile_definitions(lorawan PRIVATE ${TLNS_DEF})
	target_include_directories(lorawan PRIVATE "." "third-party" ${VCPKG_INC} ${Intl_INCLUDE_DIRS})
	target_link_libraries(lorawan PRIVATE ${OS_SPECIFIC_LIBS} )

	#
	# tlns-check
	#
	set(CLI_MAIN_CHECK_SRC
		cli-main-check.cpp
		task-response-threaded.cpp
		${ARGTABLE}
	)

	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		# Semtech's gateway source does not Windows compatible yet
		set(LIBLORAGW_INC third-party/libloragw-win)
	else ()
		set(LIBLORAGW_INC third-party/libloragw third-party/libloragw/inc)
		add_executable(tlns-check ${CLI_MAIN_CHECK_SRC}
			third-party/libloragw/subst-call-c.cpp third-party/libloragw/libloragw-helper.cpp
		)
		target_link_libraries(tlns-check PRIVATE lorawan ${OS_SPECIFIC_LIBS} ${BACKEND_DB_LIB} ${LIBINTL} ${LIBUVA} )
		target_compile_definitions(tlns-check PRIVATE ${TLNS_DEF})
		target_include_directories(tlns-check PRIVATE "." "third-party" "third-party/libloragw" "third-party/libloragw/inc" "gw-dev/usb"
				${VCPKG_INC} ${Intl_INCLUDE_DIRS})

	endif ()

	#
	# Semtech's gateway JSON config file to c++ source convert utility
	#
	add_executable(gateway-config2cpp
		util/gateway-config2cpp.cpp lorawan/proto/gw/gateway-file-json.cpp
		${ARGTABLE}
	)
	target_link_libraries(gateway-config2cpp PRIVATE lorawan ${OS_SPECIFIC_LIBS} ${LIBINTL})
	target_compile_definitions(gateway-config2cpp PRIVATE ${TLNS_DEF})
	target_include_directories(gateway-config2cpp PRIVATE "." "third-party" ${LIBLORAGW_INC}
		${VCPKG_INC} ${Intl_INCLUDE_DIRS}
	)

	#
	# Regional parameters JSON file to c++ 11 source convert utility
	#
	add_executable(regional-parameters2cpp
		util/regional-parameters2cpp.cpp
		lorawan/regional-parameters/regional-parameter-channel-plan-mem.cpp
		${ARGTABLE}
	)
	target_link_libraries(regional-parameters2cpp PRIVATE lorawan ${OS_SPECIFIC_LIBS} ${LIBINTL})
	target_compile_definitions(regional-parameters2cpp PRIVATE ${TLNS_DEF})
	target_include_directories(regional-parameters2cpp PRIVATE "." "third-party" "third-party/libloragw" "third-party/libloragw/inc"
			${VCPKG_INC} ${Intl_INCLUDE_DIRS}
	)

	#
	#  LoRaWAN node device simulator
	#
	add_executable(wire-send
		cli-wire-send.cpp
		${ARGTABLE}
	)
	target_link_libraries(wire-send PRIVATE lorawan ${OS_SPECIFIC_LIBS} ${LIBINTL})
	target_compile_definitions(wire-send PRIVATE ${TLNS_DEF})
	target_include_directories(wire-send PRIVATE "." "third-party" ${VCPKG_INC} ${Intl_INCLUDE_DIRS})

	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		# avoid Error LNK2038 mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
		set(OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
	endif()

	#
	# Plugin is a bridge to the server app.
	#
	
	#
	# Plugin example: Simple output to the stdout
	#
	add_library(stdout-bridge SHARED lorawan/bridge/stdout-bridge.cpp)
	target_link_libraries(stdout-bridge PRIVATE lorawan)
	target_include_directories(stdout-bridge PRIVATE ".")
	set_target_properties(stdout-bridge PROPERTIES SOVERSION ${VERSION_INFO})

	#
	# Plugin example: Simple output to the file
	#
	add_library(file-json-bridge SHARED lorawan/bridge/file-json-bridge.cpp)
	target_link_libraries(file-json-bridge PRIVATE lorawan)
	target_include_directories(file-json-bridge PRIVATE ".")
	set_target_properties(file-json-bridge PROPERTIES SOVERSION ${VERSION_INFO})

	#
	# Plugin example: Simple output to TCP port
	#
	add_library(tcp-udp-bridge SHARED lorawan/bridge/tcp-udp-v4-bridge.cpp)
	target_link_libraries(tcp-udp-bridge PRIVATE lorawan)
	target_include_directories(tcp-udp-bridge PRIVATE ".")
	set_target_properties(tcp-udp-bridge PROPERTIES SOVERSION ${VERSION_INFO})

	install(FILES README.md LICENSE HISTORY COPYING TODO CODE_OF_CONDUCT.md CONTRIBUTING.md DESTINATION third-party/gw-regional-settings)

	include(InstallRequiredSystemLibraries)

	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
	set(CPACK_PACKAGE_VERSION_MAJOR "0")
	set(CPACK_PACKAGE_VERSION_MINOR "1")
	include(CPack)
endif()

#
# MQTT support
#
if (ENABLE_MQTT)
	message("Install Eclipse Paho MQTT C library and C++ wrapper from the source (see bridge/mqtt/README.bridge.mqtt.md)")
	add_subdirectory(bridge/mqtt)
endif()

#
# Gateway and identifier storage
#
add_subdirectory(storage)

#
# Tests
#
add_subdirectory(tests)
# add_test(NAME test-printf COMMAND "test-printf")
# add_test(NAME test-decode-rxpk COMMAND "test-decode-rxpk")
# add_test(NAME test-mac-parse COMMAND "test-mac-parse")
# add_test(NAME test-payload2device-parser COMMAND "test-payload2device-parser")

# Print config
message("")
message("Options (on- enabled, off- disabled):")
message("")
message("-DENABLE_GW_DEV_USB=${ENABLE_GW_DEV_USB} \t build RAK2287 USB gateway backend")
message("-DENABLE_LIBUV=${ENABLE_LIBUV} \t enable libuv UDP/TCP")
message("-DENABLE_DEBUG=${ENABLE_DEBUG} \t enable debugging output")
message("-DENABLE_GEN=${ENABLE_GEN} \t enable key generator (default in memory storage)")
message("-DENABLE_HTTP=${ENABLE_HTTP} \t enable HTTP service. libmicrohttpd dependency required")
message("-DENABLE_SQLITE=${ENABLE_SQLITE} \t enable SQLITE3 backend (reserved)")
message("-DENABLE_LMDB=${ENABLE_LMDB} \t enable LMDB backend")
message("-DENABLE_JSON=${ENABLE_JSON} \t build with JSON file backend")
message("-DENABLE_MQTT=${ENABLE_MQTT} \t build with MQTT bridge")
message("-DENABLE_IPV6=${ENABLE_IPV6} \t enable IPv6 (reserved)")
message("	storage:")
message("-DENABLE_HTTP=${ENABLE_HTTP} \t enable HTTP service. libmicrohttpd dependency required")
message("-DENABLE_QRCODE=${ENABLE_QRCODE} \t enable HTTP QRCode URN service")
message("-DENABLE_MINIZIP=${ENABLE_MINIZ} \t build with minizip.")
message("")
message(" Windows: with -DENABLE_SQLITE=on must provide VCPKG options like -DCMAKE_TOOLCHAIN_FILE=C:\\git\\vcpkg\\")
message("   scripts\\buildsystems\\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows")
message("")
