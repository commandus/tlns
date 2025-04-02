#
# On Windows set correct path to the VCPKG includes at line 28
# mkdir build; cd build; cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\git\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DENABLE_SQLITE=on
#
cmake_minimum_required(VERSION 3.9)

set(VERSION_INFO 1.0.3)

if(${ESP_PLATFORM})
else()
	project("gw-dev-usb" VERSION ${VERSION_INFO})
    # ------------------------ CHANGE PATH BELOW ON WINDOWS ------------------------
    set(VCPKG_INC "/git/vcpkg/installed/x64-windows/include")
    # ------------------------ CHANGE PATH ABOVE ON WINDOWS ------------------------
	set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
	include_directories(${VCPKG_INC})

	set(CMAKE_CXX_STANDARD 11)

	set(ARGTABLE "../../third-party/argtable3/argtable3.c")
	set(AES_SRC ../../third-party/system/crypto/aes.c ../../third-party/system/crypto/cmac.c)

	set(EXTRA_INC "../../third-party")
	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		# avoid Error LNK2038 mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
		set(OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
		find_library(loragw NAMES loragw PATHS "../third-party/libloragw-win/")
		set(EXTRA_INC ${EXTRA_INC} ../../third-party/libloragw-win)
	else()
		find_library(loragw NAMES loragw PATHS "../third-party/libloragw/")
		set(EXTRA_INC ${EXTRA_INC} ../../third-party/libloragw ../../third-party/libloragw/inc)
		set(EXTRA_SRC ${EXTRA_SRC}
			../../third-party/libloragw/libloragw-helper.cpp
			../../third-party/libloragw/subst-call-c.cpp
		)
	endif()

	#
	# gw-dev-usb
	#
	add_executable(gw-dev-usb
		gw-dev-usb.cpp task-usb-socket.cpp rak2287.cpp gateway-settings-helper.cpp
		${ARGTABLE}
		../../task-response-threaded.cpp
		../../lorawan/bridge/stdout-bridge.cpp
		../../lorawan/downlink/run-downlink.cpp
		../../lorawan/downlink/downlink-by-timer.cpp
		../../lorawan/bridge/stdout-bridge.cpp
		../../lorawan/helper/passphrase.cpp
		../../third-party/daemonize.cpp
		${EXTRA_SRC}
	)
	target_link_libraries(gw-dev-usb PRIVATE ${OS_SPECIFIC_LIBS} ${LIBINTL} lorawan loragw)
	target_compile_definitions(gw-dev-usb PRIVATE ${TLNS_DEF})
	target_include_directories(gw-dev-usb PRIVATE "." "../.." ${EXTRA_INC} ${VCPKG_INC} ${Intl_INCLUDE_DIRS})

	#
	# listen-lora-usb
	#

	add_executable(listen-lora-usb
		listen-lora-usb.cpp rak2287.cpp gateway-settings-helper.cpp
		usb-listener.cpp
		${ARGTABLE}
		../../third-party/daemonize.cpp
		${EXTRA_SRC}
	)
	target_link_libraries(listen-lora-usb PRIVATE ${OS_SPECIFIC_LIBS} ${LIBINTL} lorawan loragw)
	target_compile_definitions(listen-lora-usb PRIVATE ${TLNS_DEF})
	target_include_directories(listen-lora-usb PRIVATE "." "../.." ${EXTRA_INC} ${VCPKG_INC} ${Intl_INCLUDE_DIRS})

	set(CPACK_PACKAGE_VERSION_MAJOR "0")
	set(CPACK_PACKAGE_VERSION_MINOR "1")
	include(CPack)
endif()
