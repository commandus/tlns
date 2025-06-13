#
# On Windows set correct path to the VCPKG:
# cmake -DCMAKE_TOOLCHAIN_FILE=C:/git/vcpkg/scripts/buildsystems/vcpkg.cmake ..
#
# -DENABLE_GEN=on   	enable key generator (default in memory storage)
# -DENABLE_SQLITE=off  	enable SQLITE3 backend. sqlite3 libsqlite3-dev dependencies required.
# -DENABLE_JSON=on		Build with JSON file
#
set(SRC_ROOT "../..")
set(LORAWAN ${SRC_ROOT}/lorawan)

set(CMAKE_CXX_STANDARD 11)

option(ENABLE_GEN "Build with generator" OFF)
option(ENABLE_SQLITE "Build with SQLite" OFF)
option(ENABLE_JSON "Build with JSON file" ON)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	# avoid Error LNK2038 mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
	# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
	# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
	set(OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
endif()

if (ENABLE_SQLITE)
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

add_executable(example-direct example-direct.cpp ${LORAWAN}/storage/service/gateway-service-json.cpp)
target_link_libraries(example-direct PRIVATE lorawan ${OS_SPECIFIC_LIBS} ${BACKEND_DB_LIB} lorawan)
target_include_directories(example-direct PRIVATE ${BACKEND_DB_INC})

# g++ -o example-gw-mem -I.. example-gw-mem.cpp -L../cmake-build-debug -llorawan
add_executable(example-gw-mem example-gw-mem.cpp)
target_link_libraries(example-gw-mem PRIVATE lorawan ${OS_SPECIFIC_LIBS} lorawan)

# g++ -o example-mem -I.. example-mem.cpp -L../cmake-build-debug -llorawan
add_executable(example-mem example-mem.cpp)
target_link_libraries(example-mem PRIVATE lorawan ${OS_SPECIFIC_LIBS} lorawan)

if (ENABLE_GEN)
	#
	# example-gen
	#
	# g++ -o example-gen -I.. example-gen.cpp -L../cmake-build-debug -llorawan
	add_executable(example-gen example-gen.cpp)
	target_link_libraries(example-gen PRIVATE lorawan ${OS_SPECIFIC_LIBS} lorawan)
endif()

if (ENABLE_JSON)
	#
	# example-json
	#
	add_executable(example-json example-json.cpp ${LORAWAN}/storage/service/gateway-service-json.cpp)
	target_link_libraries(example-json PRIVATE lorawan ${OS_SPECIFIC_LIBS} lorawan)
endif()

# Print config
message("")
message("Options (on- enabled, off- disabled):")
message("")
message("-DENABLE_GEN=${ENABLE_GEN} \t enable key generator (default in memory storage)")
message("-DENABLE_SQLITE=${ENABLE_SQLITE} \t enable SQLite3 backend")
message("-DENABLE_JSON=${ENABLE_JSON} \t build with JSON file backend. -DENABLE_HTTP=on required")
message("")
