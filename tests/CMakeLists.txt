# 3.9
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
	../lorawan/lorawan-types.cpp
	../lorawan/lorawan-string.cpp
	../lorawan/lorawan-conv.cpp
	../lorawan/helper/aes-helper.cpp
	../lorawan/lorawan-mac.cpp
	../lorawan/lorawan-date.cpp
	../third-party/system/crypto/aes.c
	../third-party/system/crypto/cmac.c
	../lorawan/proto/gw/proto-gw-parser.cpp
	../lorawan/proto/gw/basic-udp.cpp
	../lorawan/task/message-task-dispatcher.cpp
)
target_include_directories(test-decode-rxpk PRIVATE .. ../third-party)
target_link_libraries(test-decode-rxpk PRIVATE lorawan)

#
add_executable(test-decode-1
	test-decode-1.cpp
	../lorawan/lorawan-types.cpp
	../lorawan/lorawan-string.cpp
	../lorawan/lorawan-conv.cpp
	../lorawan/helper/aes-helper.cpp
	../lorawan/lorawan-mac.cpp
	../lorawan/lorawan-date.cpp
	../third-party/system/crypto/aes.c
	../third-party/system/crypto/cmac.c
	../lorawan/proto/gw/proto-gw-parser.cpp
	../lorawan/proto/gw/basic-udp.cpp
	../lorawan/task/message-task-dispatcher.cpp
)
target_include_directories(test-decode-1 PRIVATE .. ../third-party)
target_link_libraries(test-decode-1 PRIVATE lorawan)

#
add_test(NAME test-printf COMMAND "test-printf")
add_test(NAME test-decode-rxpk COMMAND "test-decode-rxpk")
add_test(NAME test-decode-1 COMMAND "test-decode-1")
