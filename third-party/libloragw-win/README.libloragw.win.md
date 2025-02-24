# libloragw-win

SX1302 LoRa Gateway core library [SX1302 LoRa Gateway project](https://github.com/Lora-net/sx1302_hal/) Windows version
for RAK2287 USB gateway(no GPS) connects to the Windows host's USB port. 

## Build

Make sure CMake, Git, essential build tools are installed.

```
cd libloragw-win
mkdir build
cd build
cmake ..
make
```

### Test RAK2287

Insert RAK2287 PCI to USB adapter into Windows PC's USB port.

Check if COM3 (or COM4,..) ports are available.

Run test program with argument "COM3" (or COM4,..)

```
cd tests
.\test-usb-init COM3
```

Turn on some LoRaWAN device.

Check received packets.
