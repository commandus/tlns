project(bridge-mqtt-wss)
cmake_minimum_required(VERSION 3.9)

set(PAHO_ROOT "/home/andrei/git/paho.mqtt.cpp")
set(PAHO_INC "${PAHO_ROOT}/include")
set(PAHO_LIB "${PAHO_ROOT}/build/src/libpaho-mqttpp3.so")

set(VERSION_INFO 1.0.1)

set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#add_executable(test-tlns-mqtt-ws bridge-mqtt-wss.cpp)
#target_link_libraries(test-tlns-mqtt-ws ${PAHO_LIB} paho-mqtt3as)
#target_include_directories(test-tlns-mqtt-ws PRIVATE ${PAHO_INC})

add_library(bridge-mqtt-wss SHARED bridge-mqtt-wss.cpp)
target_link_libraries(bridge-mqtt-wss PRIVATE ../../lorawan)
target_include_directories(bridge-mqtt-wss PRIVATE "." "../..")
set_target_properties(bridge-mqtt-wss PROPERTIES SOVERSION ${VERSION_INFO})

