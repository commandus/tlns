# MQTT secure websocket bridge

Secure websocket MQTT transport (wss://) bridge sample implementation.

## Dependencies

Requires Eclipse Paho MQTT C library and C++ wrapper.

Ubuntu repository contain too old C++ wrapper library it  
does not support websocket at all.

You need both C library and C++ wrapper library.

- install Eclipse Paho MQTT C library from repository first and
- build C++ wrapper from the source to make sure it supports websockets

From console:

```
sudo apt install libpaho-mqtt-dev
cd ~/git/
git clone https://github.com/eclipse/paho.mqtt.cpp
cd paho.mqtt.cpp
mkdir build
cd build
cmake ..
make
sudo make install
```

## Check CMakeLists.txt

Replace path "/home/andrei/git/" to valid path to the paho.mqtt.cpp directory in the CMakeLists.txt   
```
set(PAHO_ROOT "/home/andrei/git/paho.mqtt.cpp")
```

## Build

Build using option:

```
cd build
cmake -DENABLE_MQTT=on ..
make
```

or build separately:

```
cd bridge/mqtt
mkdir build
cd build
cmake ..
make
```
