#
# On Windows set correct path to the VCPKG:
# cmake -DCMAKE_TOOLCHAIN_FILE=C:/git/vcpkg/scripts/buildsystems/vcpkg.cmake ..
#
# Options:
# -DENABLE_LIBUV=off   	enable libuv UDP/TCP. Default UDP only. libuv dependency required.
# -DENABLE_DEBUG=off   	enable debugging output
# -DENABLE_GEN=on   	enable key generator (default in memory storage)
# -DENABLE_HTTP=off		enable HTTP service. libmicrohttpd dependency required.
# -DENABLE_QRCODE=off	enable HTTP QRCode URN service.
# -DENABLE_SQLITE=off  	enable SQLITE3 backend. sqlite3 libsqlite3-dev dependencies required.
# -DENABLE_LMDB=off  	enable LMDB backend. liblmdb++-dev (and liblmdb-dev) dependencies required.
# -DENABLE_IPV6=off   	enable IPv6 (reserved)
# -DENABLE_JSON=off		Build with JSON file
# -DENABLE_MINIZIP=off	Build with minizip
#
# Example:
# cmake -DENABLE_LIBUV=on
#
cmake_minimum_required(VERSION 3.9)

set(VERSION_INFO 1.0.3)

set(SRC_ROOT "..")
set(LORAWAN ${SRC_ROOT}/lorawan)

if(${ESP_PLATFORM})
    include(. ${SRC_ROOT} ${SRC_ROOT}/third-party $ENV{IDF_PATH}/tools/cmake/project.cmake)
	project("lorawan-storage" VERSION ${VERSION_INFO})
#    get_filename_component(EXTRA_COMPONENT_DIRS esp32 ABSOLUTE)
#    set(IDF_EXTRA_COMPONENT_DIRS ${EXTRA_COMPONENT_DIRS})
else()
	project("lorawan-storage" VERSION ${VERSION_INFO})
    # ------------------------ CHANGE PATH BELOW ON WINDOWS ------------------------
    set(VCPKG_INC "C:\\git\\vcpkg\\installed\\x64-windows\\include")
	set(MINGW_LIB "C:\\git\\vcpkg\\installed\\x64-windows\\lib")	# for MinGW only
    # ------------------------ CHANGE PATH ABOVE ON WINDOWS ------------------------
	set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
	include_directories(. ${SRC_ROOT} ${SRC_ROOT}/third-party ${VCPKG_INC} ${Intl_INCLUDE_DIRS})

	set(CMAKE_CXX_STANDARD 11)

	option(ENABLE_IPV6 "Build with IPv6" OFF)
	option(ENABLE_DEBUG "Build with debug" OFF)
	option(ENABLE_LIBUV "Build with libuv" OFF)
	option(ENABLE_GEN "Build with generator" OFF)
	option(ENABLE_HTTP "Build with HTTP" OFF)
	option(ENABLE_QRCODE "Build with HTTP QRCode URN" OFF)
	option(ENABLE_SQLITE "Build with SQLite" OFF)
	option(ENABLE_LMDB "Build with LMDB" OFF)
	option(ENABLE_JSON "Build with JSON file" ON)
	option(ENABLE_MINIZIP "Build with minizip" OFF)

	set(ARGTABLE ${SRC_ROOT}/third-party/argtable3/argtable3.c)
	set(AES_SRC ${SRC_ROOT}/third-party/system/crypto/aes.c third-party/system/crypto/cmac.c)

	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		# avoid Error LNK2038 mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
		set (OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
	endif()

	#
	# Plugins
	#
	add_library(storage-mem SHARED ${LORAWAN}/storage/service/identity-service-mem.cpp)
	target_link_libraries(storage-mem PRIVATE lorawan)
	target_include_directories(storage-mem PRIVATE ".")
	set_target_properties(storage-mem PROPERTIES SOVERSION ${VERSION_INFO})

	if (ENABLE_JSON)
		set(STORAGE_JSON ${LORAWAN}/storage/service/identity-service-json.cpp ${LORAWAN}/storage/service/gateway-service-json.cpp)
		add_library(storage-json SHARED ${STORAGE_JSON})
		target_link_libraries(storage-json PRIVATE lorawan)
		set_target_properties(storage-json PROPERTIES SOVERSION ${VERSION_INFO})
	endif()

	add_library(storage-gen SHARED ${LORAWAN}/storage/service/identity-service-gen.cpp)
	target_link_libraries(storage-gen PRIVATE lorawan)
	set_target_properties(storage-gen PROPERTIES SOVERSION ${VERSION_INFO})

	if (ENABLE_SQLITE)
		add_library(storage-sqlite SHARED
				${LORAWAN}/storage/service/identity-service-sqlite.cpp
				${LORAWAN}/storage/service/gateway-service-sqlite.cpp
		)
		target_link_libraries(storage-sqlite PRIVATE lorawan ${BACKEND_DB_LIB})
		target_include_directories(storage-sqlite PRIVATE ".")
		set_target_properties(storage-sqlite PROPERTIES SOVERSION ${VERSION_INFO})
	endif()

	if (ENABLE_LMDB)
		add_library(storage-lmdb SHARED
				${LORAWAN}/storage/service/identity-service-lmdb.cpp
				${LORAWAN}/storage/service/gateway-service-lmdb.cpp
		)
		target_link_libraries(storage-lmdb PRIVATE lorawan ${BACKEND_DB_LIB})
		target_include_directories(storage-lmdb PRIVATE ".")
		set_target_properties(storage-lmdb PROPERTIES SOVERSION ${VERSION_INFO})
	endif()

	add_library(storage-udp SHARED ${LORAWAN}/storage/service/identity-service-udp.cpp)
	target_link_libraries(storage-udp PRIVATE lorawan)
	target_include_directories(storage-udp PRIVATE ".")
	set_target_properties(storage-udp PROPERTIES SOVERSION ${VERSION_INFO})

	#
	# C to C++ connector to identity serialization routines
	#
	add_library(identity-c SHARED
		${LORAWAN}/storage/service/identity-service-c-wrapper.cpp
		${LORAWAN}/wrapper/connector-identity-serialization.cpp
	)
	target_link_libraries(identity-c PRIVATE lorawan ${BACKEND_DB_LIB})
	set_target_properties(identity-c PROPERTIES SOVERSION ${VERSION_INFO})
	target_compile_definitions(identity-c PRIVATE ${TLNS_DEF})

	#
	# lorawan-identity-service
	#
	set(LORAWAN_IDENTITY_SERVICE_SRC
		cli-main.cpp cli-helper.cpp
		${SRC_ROOT}/third-party/daemonize.cpp
		${LORAWAN}/storage/network-identity.cpp
		${ARGTABLE}
	)

	add_executable(lorawan-identity-service ${LORAWAN_IDENTITY_SERVICE_SRC})
	target_link_libraries(lorawan-identity-service PRIVATE ${OS_SPECIFIC_LIBS} ${BACKEND_DB_LIB} ${LIBMICROHTTPD} ${LIBINTL} ${LIBUVA} lorawan)
	target_compile_definitions(lorawan-identity-service PRIVATE ${TLNS_DEF})

	#
	# lorawan-identity-query
	#
	add_executable(lorawan-identity-query cli-query-main.cpp cli-helper.cpp
		${STORAGE_JSON}
		${LORAWAN}/storage/network-identity.cpp
		${ARGTABLE}
	)
	target_link_libraries(lorawan-identity-query PRIVATE ${OS_SPECIFIC_LIBS} ${LIBINTL} ${LIBUVA} lorawan)
	target_compile_definitions(lorawan-identity-query PRIVATE ${TLNS_DEF})

	#
	# lorawan-query-identity-direct
	#
	add_executable(lorawan-query-identity-direct cli-query-plugin-main.cpp cli-helper.cpp
		${LORAWAN}/storage/network-identity.cpp
		${STORAGE_JSON}
		${ARGTABLE}
	)
	target_link_libraries(lorawan-query-identity-direct PRIVATE ${OS_SPECIFIC_LIBS} ${LIBINTL} ${BACKEND_DB_LIB} lorawan)
	target_compile_definitions(lorawan-query-identity-direct PRIVATE ${TLNS_DEF})

	#
	# lorawan-identity-print
	#
	add_executable(lorawan-identity-print cli-print.cpp cli-helper.cpp
		${STORAGE_JSON}
		${ARGTABLE}
	)
	target_link_libraries(lorawan-identity-print PRIVATE ${OS_SPECIFIC_LIBS} ${LIBINTL} ${BACKEND_DB_LIB} lorawan)
	target_include_directories(lorawan-identity-print PRIVATE ${BACKEND_DB_INC})

	#
	# lorawan-tag
	#
	add_executable(lorawan-tag cli-tag.cpp cli-helper.cpp ${ARGTABLE})
	target_link_libraries(lorawan-tag PRIVATE ${OS_SPECIFIC_LIBS} ${LIBINTL} lorawan)
	target_compile_definitions(lorawan-tag PRIVATE ${TLNS_DEF})

	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		# avoid Error LNK2038 mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
		set(OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
	endif()

	install(FILES README.storage.md LICENSE HISTORY COPYING TODO CODE_OF_CONDUCT.md CONTRIBUTING.md DESTINATION systemd)

	include(InstallRequiredSystemLibraries)

	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
	set(CPACK_PACKAGE_VERSION_MAJOR "0")
	set(CPACK_PACKAGE_VERSION_MINOR "1")
	include(CPack)

	# Print config
	message("")
	message("Options (on- enabled, off- disabled):")
	message("")
	message("-DENABLE_LIBUV=${ENABLE_LIBUV} \t enable libuv UDP/TCP")
	message("-DENABLE_DEBUG=${ENABLE_DEBUG} \t enable debugging output")
	message("-DENABLE_GEN=${ENABLE_GEN} \t enable key generator (default in memory storage)")
	message("-DENABLE_HTTP=${ENABLE_HTTP} \t enable HTTP service. libmicrohttpd dependency required")
	message("-DENABLE_QRCODE=${ENABLE_QRCODE} \t enable HTTP QRCode URN service")
	message("-DENABLE_SQLITE=${ENABLE_SQLITE} \t enable SQLite3 backend")
	message("-DENABLE_LMDB=${ENABLE_LMDB} \t enable LMDB backend")
	message("-DENABLE_IPV6=${ENABLE_IPV6} \t enable IPv6 (reserved)")
	message("-DENABLE_JSON=${ENABLE_JSON} \t build with JSON file backend. -DENABLE_HTTP=on required")
	message("-DENABLE_MINIZIP=${ENABLE_MINIZ} \t build with minizip.")
	message("")

	add_subdirectory(tests)
endif()
