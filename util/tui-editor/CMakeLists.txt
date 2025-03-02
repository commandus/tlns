#
# cmake -DCMAKE_TOOLCHAIN_FILE=C:\git\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
# Windows: install pdcurses
# cd vcpkg
# vcpkg install --triplet x64-windows pdcurses
#
cmake_minimum_required(VERSION 3.9)
set(VERSION_INFO 1.0.0)
project(view-identity VERSION ${VERSION_INFO})

set(CMAKE_CXX_STANDARD 11)

# ------------------------ CHANGE PATH ABOVE ON WINDOWS ------------------------
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
include_directories(${VCPKG_INC})

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    find_package(unofficial-pdcurses CONFIG REQUIRED)
else()
    find_library(ncurses)
endif()

if(ENABLE_HTTP)
    find_library(LIBMICROHTTPD NAMES microhttpd libmicrohttpd-dll.lib HINTS /usr/lib/x86_64-linux-gnu/ ${VCPKG_LIB}    )
    set(EXTRA_LIBS ${EXTRA_LIBS} ${LIBMICROHTTPD})
endif()

if (ENABLE_SQLITE)
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        find_package(unofficial-sqlite3 CONFIG REQUIRED)
        set(EXTRA_LIBS ${EXTRA_LIBS} unofficial::sqlite3::sqlite3)
    else ()
        find_package(SQLite3 REQUIRED)
        set(EXTRA_LIBS ${EXTRA_LIBS} ${SQLite3_LIBRARIES})
    endif()
    set(EXTRA_DEF ${EXTRA_DEF} ENABLE_SQLITE)
endif()

if (ENABLE_LMDB)
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        find_package(unofficial-lmdb CONFIG REQUIRED)
        set(EXTRA_LIBS ${EXTRA_LIBS} unofficial::lmdb::lmdb)
    else ()
        find_library(LIBLMDB NAMES lmdb)
        set(EXTRA_LIBS ${EXTRA_LIBS} ${LIBLMDB})
    endif ()
endif()

add_executable(view-identity view-identity.cpp
    identity-editor.cpp
    curses-identity-editor.cpp
)
target_include_directories(view-identity PRIVATE . ../..)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(view-identity PRIVATE unofficial::pdcurses::pdcurses lorawan wsock32 ws2_32 Userenv ${EXTRA_LIBS})
else()
    target_link_libraries(view-identity PRIVATE ncurses lorawan ${EXTRA_LIBS})
endif()

target_compile_definitions(view-identity PRIVATE ${EXTRA_DEF})

add_subdirectory(tests)
