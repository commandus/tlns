# RAK2287 USB gateway 

## Preparation

Windows requires CP210xUSB to UART bridge VCP driver installed.

See [https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers](https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers)

After COM port number re-assignment and possibly other manipulation driver may require re-install.

To do this, run:

```
cd "C:\Program Files\DIFX\4A7292F75FEBBD3C\" 
CP210xVCPInstaller_x64.exe /u C:\WINDOWS\System32\DriverStore\FileRepository\slabvcp.inf_amd64_ab8310f5de07b344\slabvcp.inf
```

or find out other way to uninstall driver.

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

## Allow non-root access to /dev/ttyACM0

Unable to access the device connected in /dev/ttyACM0 0 you get message:

```
failed to open COM port /dev/ttyACM0 - Permission denied
```

Add user to the 'dialout' group:

```
sudo usermod -a -G dialout $USER 
```

Reboot computer.
