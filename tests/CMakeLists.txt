cmake_minimum_required(VERSION 3.9)

set(VERSION_INFO 1.0.3)

project("tlns-test" VERSION ${VERSION_INFO})

add_executable(test-printf
	test-printf.cpp
	../third-party/libloragw/subst-call-c.cpp
	../third-party/libloragw/libloragw-helper.cpp
)
target_include_directories(test-printf PRIVATE .. ../third-party/libloragw ../gw-dev/usb)


add_executable(test-decode-rxpk
	test-decode-rxpk.cpp
)
target_include_directories(test-decode-rxpk PRIVATE .. ../third-party)


add_test(NAME test-printf COMMAND "test-printf")
add_test(NAME test-decode-rxpk COMMAND "test-decode-rxpk")
