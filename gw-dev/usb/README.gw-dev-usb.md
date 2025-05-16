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

## Send downlink message to the end-device

### STM32WL55JC

- build example (STM32CubeMX and STM32CubeIDE)
- run example


#### Build example 

##### Run STM32CubeMX

Select the example for the debug board (enter wl55 in the search).

Select the example Lorawan_AT_Slave.

Select Open in STM32CubeIDE.

Select and remember the directory in which the example will be loaded.

#####  Run STM32CubeIDE

Select the project in the directory where the example was created.

Run. Update firmware if prompted.

#####  Connect to STM32WL55JC

In the console, connect to COM8 (or other) port and enter

```
AT+JOIN=0
```

to select ABP.

The console displays the address and other parameters, for example:

```
AppKey:      2B:7E:15:16:28:AE:D2:A6:AB:F7:15:88:09:CF:4F:3C
NwkKey:      2B:7E:15:16:28:AE:D2:A6:AB:F7:15:88:09:CF:4F:3C
AppSKey:     2B:7E:15:16:28:AE:D2:A6:AB:F7:15:88:09:CF:4F:3C
NwkSKey:     2B:7E:15:16:28:AE:D2:A6:AB:F7:15:88:09:CF:4F:3C
DevEUI:      00:80:E1:15:00:0A:95:7B
AppEUI:      01:01:01:01:01:01:01:01
DevAddr:     00:0A:95:7B
```

Generate valid radio packet for address 00:0A:95:7B and AppSKey, NwkSKey.

```
607B950A00000000026b69636b6173732d776f7a6e69616bD0955D84
```

Turn on class C:
```
AT+CLASS=C
```

## Send radio packet

Send radio packet in RX1 and RX2 window:

```
send-lora-usb -1 -2 -c EU863-870 COM3 -p 607B950A00000000026b69636b6173732d776f7a6e69616bD0955D84
```

Then send downlink message 'aabbccdd' to FPort 2 without confirmation (0- no confirmation, 1- with confirmation):

```
AT+SEND=2:0:aabbccdd
```

Check if messages have been sent in RX1, RX2 time window and received by the device.

Send the next packets by correcting the FCnt value and recalculating the MIC (see table below)

| FCnt | Radio packet                                             |
|------|----------------------------------------------------------|
| 0    | 607B950A00000000023F0982E42ACE583507B7EB80BC7DA32F968FC3 |
| 1    | 607B950A00000100023F0982E42ACE583507B7EB80BC7DA3E876088F |
| 2    | 607B950A00000200023F0982E42ACE583507B7EB80BC7DA3A1B51F60 |
| 3    | 607B950A00000300023F0982E42ACE583507B7EB80BC7DA3B091009C |
| 4    | 607B950A00000400023F0982E42ACE583507B7EB80BC7DA371438344 |
| 5    | 607B950A00000500023F0982E42ACE583507B7EB80BC7DA3BD56EBF6 |

### Heltec OLED

| FCnt | Radio packet                                               |
|------|------------------------------------------------------------|
| 1    | 60e16a7e00000100023F0982E42ACE583507B7EB80BC7DA396CFA76C   |
| 2    | 60e16a7e00000200023F0982E42ACE583507B7EB80BC7DA3419B42AD   | 
| 3    | 60e16a7e00000300023F0982E42ACE583507B7EB80BC7DA32C45AB6E   |
| 4    | 60e16a7e00000400023F0982E42ACE583507B7EB80BC7DA375A3A230   |
| 5    | 60e16a7e00000500023F0982E42ACE583507B7EB80BC7DA38387FA21   |
| 6    | 60e16a7e00000600023F0982E42ACE583507B7EB80BC7DA341912A6A   |
