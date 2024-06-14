# RAK2287 USB gateway 


## Acknowledgement

Based on lora_pkt_fwd.c (C)2019 Semtech License: Revised BSD License, see LICENSE.TXT file include in the project

Classes:

```
MessageTaskDispatcher::sockets.push_back(new TaskUsbGatewayUnixSocket) add input/output socket 
    where socket class
    TaskUsbGatewayUnixSocket (TaskSocket) 
        LoraGatewayListener member wraps device specific code (based on Semtech's lora_pkt_fwd.c)     
```
