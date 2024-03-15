# libloragw

Part of [lorawan-network-server](https://github.com/commandus/lorawan-network-server.git).

You can compile lorawan-network-server with embedded gateway. Gateway RAK2287 connects to the host's USB port. 

lorawan-network-server штсдгву standalone lorawan-gateway as an option.

lorawan-network-server's Automake Makefile.am search for libloragw.a in ../libloragw or ../libloragw/build 
directories.

First of all you need RAK2287 PCI USB, no GPS version with USB adapter.

Then clone [rak_common_for_gateway](https://github.com/RAKWireless/rak_common_for_gateway.git) repository.

This project rely on rak_common_for_gateway sources.

This project build libloragw with replaced open(), close(), printf()/fprintf() calls with macros:

```
#define open open_c
#define close close_c
#define printf printf_c
#define fprintf(fd, args...) printf_c(args)
```

Then in the lorawan-network-server's subst-call-c.cpp implements functions:

- open_c()
- close_c()
- printf_c()

printf_c() send output to the log file instead of stdost/stdxerr as printf() does.

open_c() and close_c() open and close USB COM port.

## Prerequsites

### Install RAK2287

Clone repository

```
cd ~/git
git clone https://github.com/RAKWireless/rak_common_for_gateway.git
```

Compile RAK2287

Linux:
```
cd ~/git/rak_common_for_gateway
sudo ./install.sh

In the menu choose 9) RAK2287 USB (enter 9)
```

Stop running chirp-* services.

On Windows, run it in WSL2 Ubuntu then copy rak_common_for_gateway/ to the Windows folder git.

### Test RAK2287

Do not test on Windows.

Run packet forwarder:
```
cd ~/git/1/rak_common_for_gateway/lora/rak2287/packet_forwarder/lora_pkt_fwd
./lora_pkt_fwd
```

## Build

Make sure CMake, Git, essential build tools are installed.

Go out from the lorawan-network-server source directory:
```
cd ..
```

Clone repository in the parent directory of the lorawan-network-server source directory:
```
git clone https://github.com/commandus/libloragw.git
```

Check path to RAK2287 libloragw lib sources in the CMakeLists.txt:
```
vi CMakeLists.txt
```

Find line

```
set(RAK_COMMON_FOR_GATEWAY_ROOT ../../rak_common_for_gateway)
```

and replace with actual path of RAK root directory.


Build library:

```
cd libloragw
mkdir build
cd build
cmake ..
make
```

