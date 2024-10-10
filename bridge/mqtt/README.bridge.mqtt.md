# MQTT secure websocket bridge

Secure websocket MQTT transport (wss://) application bridge sample implementation.

## Dependencies

Requires Eclipse Paho MQTT C library and C++ wrapper.

Ubuntu repository contain too old C++ wrapper library it  
does not support websocket at all.

You need both C library and C++ wrapper library.

- install Eclipse Paho MQTT C library from repository first and
- build C++ wrapper from the source to make sure it supports websockets

Run from the command line:

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

## Check CMakeLists.txt or Makefile.am

If you are using CMake, replace path "/home/andrei/git/" to valid path to the paho.mqtt.cpp directory in the CMakeLists.txt   
```
set(PAHO_ROOT "/home/andrei/git/paho.mqtt.cpp")
```

If you are using GNU Autoconf/Automake, replace path "/home/andrei/git/" to valid path to the paho.mqtt.cpp directory in the Makefile.am
```
    PAHO_ROOT = /home/andrei/git/paho.mqtt.cpp
```

## Build

### Build using CMake

Build using option ENABLE_MQTT in the main CMakeLists.txt:

```
mkdir -p build
cd build
cmake -DENABLE_MQTT=on ..
make
```

or build separately using bridge/CMakeLists.txt:

```
cd bridge/mqtt
mkdir -p build
cd build
cmake ..
make
```

### Build using GNU Autoconf/Automake

Configure with option --enable-mqtt:

```shell
./autogen.sh
./configure --enable-mqtt
make
```
