# RAK2287 USB gateway 


## Acknowledgement

Based on lora_pkt_fwd.c (C)2019 Semtech License: Revised BSD License, see LICENSE.TXT file include in the project

Classes:

```
MessageTaskDispatcher::sockets.push_back(new TaskUsbGatewaySocket) add input/output socket 
    where socket class
    TaskUsbGatewaySocket (TaskSocket) 
        LoraGatewayListener member wraps device specific code (based on Semtech's lora_pkt_fwd.c)     
```

Linux
```
cd gw-dev/usb
./gw-dev-usb -c EU-863-870 -I ../../storage/libstorage-json.so -o ../.. -i identity.json -g gateway.json -vvvvvvv /dev/ttyACM0
```
Windows
```
cd gw-dev\usb
gw-dev-usb -c EU-863-870 -I ..\..\storage\storage-json.dll -o ..\.. -i identity.json -g gateway.json -vvvvvvv COM3
```

## 
```
failed to open COM port /dev/ttyACM0 - Permission denied
```

sudo usermod -a -G dialout $USER 