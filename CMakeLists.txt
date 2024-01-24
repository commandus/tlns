#
# On Windows set correct path to the VCPKG includes at line 28
# mkdir build; cd build; cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\git\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DENABLE_SQLITE=on
#
# Options:
# -DENABLE_LIBUV=off   	enable libuv UDP/TCP. Default UDP only
# -DENABLE_DEBUG=off   	enable debugging output
# -DENABLE_GEN=on   	enable key generator (default in memory storage)
# -DENABLE_SQLITE=off  	enable SQLITE3 backend
# -DJSON=off   	        enable JSON file backend
# -DENABLE_IPV6=off   	enable IPv6 (reserved)
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

	set(SRC_LIBLORAWAN
		lorawan/lorawan-conv.cpp lorawan/lorawan-date.cpp lorawan/lorawan-error.cpp lorawan/lorawan-mac.cpp
		lorawan/lorawan-msg.cpp lorawan/lorawan-string.cpp lorawan/lorawan-types.cpp
		lorawan/lorawan-packet-storage.cpp
		lorawan/helper/ip-address.cpp
		lorawan/proto/gw/basic-udp.cpp
		lorawan/storage/gateway-identity.cpp lorawan/storage/network-identity.cpp
		lorawan/task/message-queue-item.cpp lorawan/task/message-queue.cpp lorawan/task/task-descriptor.cpp
		lorawan/task/message-task-dispatcher.cpp lorawan/task/task-response.cpp
		third-party/strptime.cpp
		third-party/base64/base64.cpp
		${AES_SRC}
	)

	if (ENABLE_IPV6)
		set(TLNS_DEF ENABLE_IPV6 )
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
	endif()

	if (ENABLE_SQLITE)
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
		set(TLNS_DEF ${TLNS_DEF} ENABLE_JSON)
	endif()

	#
	# tlns-check
	#
	set(CLI_MAIN_CHECK_SRC
		cli-main-check.cpp
		${SRC_LIBLORAWAN}
		${ARGTABLE}
		task-response-threaded.cpp
	)
	add_executable(tlns-check ${CLI_MAIN_CHECK_SRC})
	target_link_libraries(tlns-check PRIVATE ${OS_SPECIFIC_LIBS} ${BACKEND_DB_LIB} ${LIBINTL} ${LIBUVA})
	target_compile_definitions(tlns-check PRIVATE ${TLNS_DEF})
	target_include_directories(tlns-check PRIVATE "." "third-party" ${VCPKG_INC} ${Intl_INCLUDE_DIRS})

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
