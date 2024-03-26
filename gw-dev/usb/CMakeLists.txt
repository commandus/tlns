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

	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)

	set(ARGTABLE "../../third-party/argtable3/argtable3.c")
	set(AES_SRC ../../third-party/system/crypto/aes.c ../../third-party/system/crypto/cmac.c)

	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		# avoid Error LNK2038 mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
		set (OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
	endif()

	set(SRC_LIBLORAWAN
		../../lorawan/lorawan-conv.cpp
		../../lorawan/lorawan-date.cpp
		../../lorawan/lorawan-error.cpp
		../../lorawan/lorawan-mac.cpp
		../../lorawan/lorawan-msg.cpp
		../../lorawan/lorawan-string.cpp
		../../lorawan/lorawan-types.cpp
		../../lorawan/lorawan-packet-storage.cpp
		../../lorawan/helper/ip-address.cpp
		../../lorawan/proto/gw/proto-gw-parser.cpp
		../../lorawan/proto/gw/basic-udp.cpp
		../../lorawan/storage/gateway-identity.cpp
		../../lorawan/storage/network-identity.cpp
		../../lorawan/task/message-queue-item.cpp
		../../lorawan/task/message-queue.cpp
		../../lorawan/task/task-descriptor.cpp
		../../lorawan/task/message-task-dispatcher.cpp
		../../lorawan/task/task-response.cpp
		../../lorawan/task/task-socket.cpp
		../../third-party/strptime.cpp
		../../third-party/base64/base64.cpp
		${AES_SRC}
	)

	add_library(lorawan ${SRC_LIBLORAWAN})
	target_link_libraries(lorawan PRIVATE ${OS_SPECIFIC_LIBS})
	target_include_directories(lorawan PRIVATE "." "../../" "../../third-party")

	find_library(loragw NAMES loragw PATHS "../third-party/libloragw/")

	#
	# gw-dev-usb
	#
	set(GW_DEV_USB_SRC
        gw-dev-usb.cpp
		${ARGTABLE}
		../../daemonize.cpp
		../../lorawan/helper/file-helper.cpp
		../../task-response-threaded.cpp
		../../third-party/libloragw/libloragw-helper.cpp
	)
	add_executable(gw-dev-usb ${GW_DEV_USB_SRC})
	set_property(TARGET gw-dev-usb PROPERTY C_STANDARD 99)
	target_link_libraries(gw-dev-usb PRIVATE ${OS_SPECIFIC_LIBS} ${LIBINTL} lorawan loragw)
	target_compile_definitions(gw-dev-usb PRIVATE ${TLNS_DEF})
	target_include_directories(gw-dev-usb PRIVATE "." "../../" "../../third-party" 
		"../../third-party/libloragw"
		"../../third-party/libloragw"
		"../../third-party/libloragw/inc"
		${VCPKG_INC} ${Intl_INCLUDE_DIRS})

	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		# avoid Error LNK2038 mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
		set(OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
	endif()

	set(CPACK_PACKAGE_VERSION_MAJOR "0")
	set(CPACK_PACKAGE_VERSION_MINOR "1")
	include(CPack)
endif()
