#
# On Windows set correct path to the VCPKG includes at line 28
# mkdir build; cd build; cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\git\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DENABLE_SQLITE=on
#
# Options:
# -DENABLE_GW_DEV_USB=off   build RAK2287 USB gateway backend
# -DENABLE_LIBUV=off   		enable libuv UDP/TCP. Default UDP only
# -DENABLE_DEBUG=off   		enable debugging output
# -DENABLE_GEN=on   		enable key generator (default in memory storage)
# -DENABLE_SQLITE=off  		enable sqlite3 backend
# -DJSON=off   	        	enable JSON file backend
# -DENABLE_IPV6=off   		enable IPv6 (reserved)
#
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
	find_package(Intl)
	find_package(Gettext)
	if(Intl_LIBRARY)
		set(LIBINTL Intl::Intl)
	endif()
	link_directories(${Intl_LIBRARY_DIRS})
	find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
	if(NOT GETTEXT_MSGFMT_EXECUTABLE)
		message("NOTE: msgfmt not found. Translations will *not* be installed")
	else()
		# Build the .po files
		set(PO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/po)
		# Input files
		file(GLOB PO_FILES ${PO_PATH}/*.po)
		# Generated file list
		set(MO_FILES)
		set(PRG lorawan-service)
		set(LOCALE_INSTALL_DIR /usr/share/locale)
		foreach(_poFile ${PO_FILES})
			GET_FILENAME_COMPONENT(_poFileName ${_poFile} NAME)
			STRING(REGEX REPLACE "^${PRG}\\." "" _langCode ${_poFileName} )
			STRING(REGEX REPLACE "\\.UTF-8\\.po$" "" _langCode ${_langCode} )
			STRING(REGEX REPLACE "_..$" "" _langCode ${_langCode} )
			if( _langCode )
				message("** PO file ${_poFileName} LANG ${_langCode} LIBDIR ${Intl_LIBRARY_DIRS} INCDIR ${Intl_INCLUDE_DIRS}")
				GET_FILENAME_COMPONENT(_lang ${_poFile} NAME_WE)

				set(_moPath ${CMAKE_CURRENT_SOURCE_DIR}/locale/${_langCode}/LC_MESSAGES)
				set(_moFile ${_moPath}/${PRG}.mo)
				message("** ** ${_moFile} install to ${LOCALE_INSTALL_DIR}/${_langCode}/LC_MESSAGES/${PRG}.mo")
				#	mkdir -p ${_moPath} ; ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_moFile} ${_poFile}
				if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
					message("** powershell ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.ps1 ${GETTEXT_MSGFMT_EXECUTABLE}
						${_moPath} ${_moFile} ${_poFile}")
					add_custom_command(OUTPUT ${_moFile}
						command powershell ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.ps1 ${GETTEXT_MSGFMT_EXECUTABLE}
							${_moPath} ${_moFile} ${_poFile} DEPENDS ${_poFile})
				else()
					message("** ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.sh ${_moPath} ${_moFile} ${_poFile}")
					add_custom_command(OUTPUT ${_moFile}
						command ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.sh ${_moPath} ${_moFile} ${_poFile}
							DEPENDS ${_poFile})
				endif()
				#			install(FILES ${_moFile} DESTINATION ${LOCALE_INSTALL_DIR}/${_langCode}/LC_MESSAGES/ RENAME ${PRG}.mo)
				list(APPEND MO_FILES ${_moFile})
			endif()
		endforeach(_poFile ${PO_FILES})
		add_custom_target(translations ALL DEPENDS ${MO_FILES})
	endif()

	set(CMAKE_CXX_STANDARD 11)

	option(ENABLE_GW_DEV_USB "Build RAK2287 USB gateway backend" ON)
	option(ENABLE_IPV6 "Build with IPv6" OFF)
	option(ENABLE_DEBUG "Build with debug" OFF)
	option(ENABLE_LIBUV "Build with libuv" OFF)
	option(ENABLE_GEN "Build with generator" OFF)
	option(ENABLE_SQLITE "Build with SQLite" OFF)
	option(ENABLE_JSON "Build with JSON file" ON)

	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)

	set(ARGTABLE "third-party/argtable3/argtable3.c")
	set(AES_SRC third-party/system/crypto/aes.c third-party/system/crypto/cmac.c)

	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		# avoid Error LNK2038 mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
		set (OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
	endif()

	add_subdirectory("third-party/libloragw")

	set(SRC_LIBLORAWAN
		lorawan/lorawan-conv.cpp lorawan/lorawan-date.cpp lorawan/lorawan-error.cpp lorawan/lorawan-mac.cpp
		lorawan/lorawan-msg.cpp lorawan/lorawan-string.cpp lorawan/lorawan-types.cpp
		lorawan/lorawan-packet-storage.cpp
		lorawan/helper/ip-address.cpp lorawan/helper/thread-helper.cpp
		lorawan/proto/gw/gw.cpp lorawan/proto/gw/proto-gw-parser.cpp lorawan/proto/gw/basic-udp.cpp
		lorawan/proto/gw/set-gateway-metadata.cpp
		lorawan/regional-parameters/regional-parameter-channel-plan.cpp lorawan/regional-parameters/regional-parameter-channel-plans.cpp
		lorawan/regional-parameters/regional-parameter-channel-plan-mem.cpp
		lorawan/storage/gateway-identity.cpp lorawan/storage/network-identity.cpp
		lorawan/task/message-queue-item.cpp lorawan/task/message-queue.cpp lorawan/task/task-descriptor.cpp
		lorawan/task/message-task-dispatcher.cpp lorawan/task/task-response.cpp
		lorawan/task/task-socket.cpp lorawan/task/task-accepted-socket.cpp
		lorawan/task/task-unix-socket.cpp
		lorawan/task/task-udp-socket.cpp lorawan/task/task-udp-control-socket.cpp
		lorawan/task/task-unix-socket.cpp lorawan/task/task-unix-control-socket.cpp
		third-party/strptime.cpp
		third-party/base64/base64.cpp
		${AES_SRC}
	)

	if (ENABLE_GW_DEV_USB)
		set(TLNS_DEF ${TLNS_DEF} ENABLE_GW_DEV_USB )
		add_subdirectory("gw-dev/usb")
	endif()

	if (ENABLE_IPV6)
		set(TLNS_DEF ${TLNS_DEF} ENABLE_IPV6 )
	endif()

	if (ENABLE_DEBUG)
		set(TLNS_DEF ${TLNS_DEF} ENABLE_DEBUG)
	endif()

	if (ENABLE_LIBUV)
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
		if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
			find_package(unofficial-sqlite3 CONFIG REQUIRED)
			set(BACKEND_DB_LIB unofficial::sqlite3::sqlite3)
		else ()
			find_package(SQLite3 REQUIRED)
			set(BACKEND_DB_LIB ${SQLite3_LIBRARIES})
			set(BACKEND_DB_INC ${SQLite3_INCLUDE_DIRS})
		endif ()
	endif()

	if (ENABLE_JSON)
		set(SRC_LIBLORAWAN ${SRC_LIBLORAWAN} lorawan/regional-parameters/regional-parameter-channel-plan-file-json.cpp)
		set(TLNS_DEF ${TLNS_DEF} ENABLE_JSON)
	endif()

	add_library(lorawan STATIC ${SRC_LIBLORAWAN})
	# -fPIC
	set_property(TARGET lorawan PROPERTY POSITION_INDEPENDENT_CODE ON)
	target_compile_definitions(lorawan PRIVATE ${TLNS_DEF})
	target_include_directories(lorawan PRIVATE "." "third-party" ${VCPKG_INC} ${Intl_INCLUDE_DIRS})

	#
	# tlns-check
	#
	set(CLI_MAIN_CHECK_SRC
		cli-main-check.cpp
		task-response-threaded.cpp
		${ARGTABLE}
	)
	add_executable(tlns-check ${CLI_MAIN_CHECK_SRC}
		third-party/libloragw/subst-call-c.cpp third-party/libloragw/libloragw-helper.cpp
	)
	target_link_libraries(tlns-check PRIVATE lorawan ${OS_SPECIFIC_LIBS} ${BACKEND_DB_LIB} ${LIBINTL} ${LIBUVA} )
	target_compile_definitions(tlns-check PRIVATE ${TLNS_DEF})
	target_include_directories(tlns-check PRIVATE "." "third-party" "third-party/libloragw" "third-party/libloragw/inc" "gw-dev/usb"
		${VCPKG_INC} ${Intl_INCLUDE_DIRS})

	add_executable(gateway-config2cpp
		tools/gateway-config2cpp.cpp
		lorawan/proto/gw/gateway-file-json.cpp
		${ARGTABLE}
	)
	target_link_libraries(gateway-config2cpp PRIVATE lorawan ${OS_SPECIFIC_LIBS} ${LIBINTL})
	target_compile_definitions(gateway-config2cpp PRIVATE ${TLNS_DEF})
	target_include_directories(gateway-config2cpp PRIVATE "." "third-party" "third-party/libloragw" "third-party/libloragw/inc"
		${VCPKG_INC} ${Intl_INCLUDE_DIRS}
	)

	add_executable(regional-parameters2cpp
		tools/regional-parameters2cpp.cpp
		lorawan/regional-parameters/regional-parameter-channel-plan-mem.cpp
		${ARGTABLE}
	)
	target_link_libraries(regional-parameters2cpp PRIVATE lorawan ${OS_SPECIFIC_LIBS} ${LIBINTL})
	target_compile_definitions(regional-parameters2cpp PRIVATE ${TLNS_DEF})
	target_include_directories(regional-parameters2cpp PRIVATE "." "third-party" "third-party/libloragw" "third-party/libloragw/inc"
			${VCPKG_INC} ${Intl_INCLUDE_DIRS}
	)

	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		# avoid Error LNK2038 mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
		# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
		set(OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
	endif()

	install(FILES README.md LICENSE HISTORY COPYING TODO CODE_OF_CONDUCT.md CONTRIBUTING.md DESTINATION doc)

	include(InstallRequiredSystemLibraries)

	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
	set(CPACK_PACKAGE_VERSION_MAJOR "0")
	set(CPACK_PACKAGE_VERSION_MINOR "1")
	include(CPack)
endif()
