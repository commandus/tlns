#
# cmake -DCMAKE_TOOLCHAIN_FILE=C:\git\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows ..
#
cmake_minimum_required(VERSION 3.9)
set(VERSION_INFO 1.0)
project("libloragw-win" VERSION ${VERSION_INFO})

set(CMAKE_CXX_STANDARD 20)

set(LIBLORAGW_SRC ../../third-party/libloragw-win)
set(JITQUEUE_SRC ../../third-party/jitqueue)
set(NANOSLEEP_SRC ../../third-party/nanosleep)

set(LIBLORAGW_INC_DIRS . ../../third-party ${LIBLORAGW_SRC})

set(LIBLORAGW_SRC
    ${LIBLORAGW_SRC}/loragw_ad5338r.c
    ${LIBLORAGW_SRC}/loragw_aux.c
    ${LIBLORAGW_SRC}/loragw_cal.c
    ${LIBLORAGW_SRC}/loragw_com.c
    ${LIBLORAGW_SRC}/loragw_gps.c
    ${LIBLORAGW_SRC}/loragw_hal.c
    ${LIBLORAGW_SRC}/loragw_lbt.c
    ${LIBLORAGW_SRC}/loragw_mcu.c
    ${LIBLORAGW_SRC}/loragw_reg.c
    ${LIBLORAGW_SRC}/loragw_stts751.c
    ${LIBLORAGW_SRC}/loragw_sx1250.c
    ${LIBLORAGW_SRC}/loragw_sx125x.c
    ${LIBLORAGW_SRC}/loragw_sx1261.c
    ${LIBLORAGW_SRC}/loragw_sx1302.c
    ${LIBLORAGW_SRC}/loragw_sx1302_rx.c
    ${LIBLORAGW_SRC}/loragw_sx1302_timestamp.c
    ${LIBLORAGW_SRC}/loragw_usb.c
    ${LIBLORAGW_SRC}/sx1250_com.c
    ${LIBLORAGW_SRC}/sx1250_usb.c
    ${LIBLORAGW_SRC}/sx125x_com.c
    ${LIBLORAGW_SRC}/sx1261_com.c
    ${LIBLORAGW_SRC}/sx1261_usb.c

    ${LIBLORAGW_SRC}/platform-win.cpp
    ${LIBLORAGW_SRC}/win-fd-com.cpp

    ${JITQUEUE_SRC}/jitqueue.cpp
    ${NANOSLEEP_SRC}/nanosleep.c
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # avoid Error LNK2038 mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
    # set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    # set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    set(OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
endif()

add_library(loragw STATIC ${LIBLORAGW_SRC})
target_include_directories(loragw PRIVATE ${LIBLORAGW_INC_DIRS})
target_link_libraries(loragw PRIVATE ${OS_SPECIFIC_LIBS})
