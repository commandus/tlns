# 3.9
cmake_minimum_required(VERSION 3.9)

set(VERSION_INFO 1.0.3)

project("tlns-test" VERSION ${VERSION_INFO})

set(CMAKE_CXX_STANDARD 20)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(INC_LIBLORAGW ../third-party/libloragw-win ../third-party/jitqueue)
else ()
	set(INC_LIBLORAGW ../third-party/libloragw ../third-party/libloragw/inc ../third-party/jitqueue)
	add_executable(test-printf
		test-printf.cpp
		../third-party/libloragw/subst-call-c.cpp
		../third-party/libloragw/libloragw-helper.cpp
	)
	target_include_directories(test-printf PRIVATE .. ../third-party/libloragw ../gw-dev/usb)
	add_test(NAME test-printf COMMAND "test-printf")
endif()

add_executable(test-decode-rxpk
	test-decode-rxpk.cpp
)
target_include_directories(test-decode-rxpk PRIVATE .. ../third-party)
target_link_libraries(test-decode-rxpk PRIVATE lorawan)

#
add_executable(test-decode-1
	test-decode-1.cpp
)
target_include_directories(test-decode-1 PRIVATE .. ../third-party)
target_link_libraries(test-decode-1 PRIVATE lorawan)


add_executable(test-mac-parse
	test-mac-parse.cpp
)
target_include_directories(test-mac-parse PRIVATE .. ../third-party)
target_link_libraries(test-mac-parse PRIVATE lorawan)

add_executable(test-payload2device-parser
	test-payload2device-parser.cpp
)
target_include_directories(test-payload2device-parser PRIVATE .. ../third-party)
target_link_libraries(test-payload2device-parser PRIVATE lorawan)


set(TEST_USB_SRC test-usb-init.cpp)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(TEST_USB_SRC ${TEST_USB_SRC})
endif()

add_executable(test-usb-init ${TEST_USB_SRC})
target_include_directories(test-usb-init PRIVATE .. ${INC_LIBLORAGW} ../third-party)
target_link_libraries(test-usb-init PRIVATE lorawan loragw)

#
add_test(NAME test-decode-rxpk COMMAND "test-decode-rxpk")
add_test(NAME test-decode-1 COMMAND "test-decode-1")
add_test(NAME test-mac-parse COMMAND "test-mac-parse")
add_test(NAME test-payload2device-parser COMMAND "test-payload2device-parser")
add_test(NAME test-usb-init COMMAND "test-usb-init")

