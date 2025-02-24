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

	set(CMAKE_CXX_STANDARD 20)

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
		set(EXTRA_SRC ${EXTRA_SRC})
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
	set(GW_DEV_USB_SRC
        gw-dev-usb.cpp task-usb-socket.cpp rak2287.cpp
		${ARGTABLE}
		../../task-response-threaded.cpp
		../../lorawan/helper/file-helper.cpp
		../../lorawan/helper/thread-helper.cpp
		../../lorawan/bridge/app-bridge.cpp
		../../lorawan/bridge/stdout-bridge.cpp
		../../lorawan/task/task-accepted-socket.cpp
		../../third-party/daemonize.cpp
		${EXTRA_SRC}
	)
	add_executable(gw-dev-usb ${GW_DEV_USB_SRC})
	target_link_libraries(gw-dev-usb PRIVATE ${OS_SPECIFIC_LIBS} ${LIBINTL} lorawan loragw)
	target_compile_definitions(gw-dev-usb PRIVATE ${TLNS_DEF})
	target_include_directories(gw-dev-usb PRIVATE "." "../.." ${EXTRA_INC} ${VCPKG_INC} ${Intl_INCLUDE_DIRS})

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
