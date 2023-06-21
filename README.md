# tlns

Tiny LoRaWAN network server project

Goal is make a small footprint running on ESP32 board.

Working release [https://github.com/commandus/lorawan-network-server](https://github.com/commandus/lorawan-network-server) runs on Raspberry. It requires much more resources.

## Tools

- Visual Code
  - Draw.io Integration Visual Code plugin

## Library

### Receiver

Receiver object try to parse received packet.

Implementation of Receiver class must override receive() method.

Receiver has interfaces to access external storages:

- get network key(address)
- get app key(address)
- get gateway(gwid)
- put MAC command
- put declined message
- put accepted message

![Receiver diagram](receiver.drawio.svg)

Fig. 1 Receiver diagram

Receiver.receive(Packet -> message queue) return accepted or declined

### Message queue

Message queue object provide

- put accepted message (used by Receiver)
- list of messages need to reply to the gateway
- list of messages ready to send to the app server
- list of messages 