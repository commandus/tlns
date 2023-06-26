# tlns

Tiny LoRaWAN Network Server

The goal of the project is to make a network server running on the ESP32 board.

The working release of [https://github.com/commandus/lorawan-network-server](https://github.com/commandus/lorawan-network-server) runs on Raspberry. This requires much more resources.

## Tools

- Visual Code
  - Draw.io Integration Visual Code plugin
  - For ESP32 install Visual Code "Espressif IDF" plugin and configure ESP-IDF 

## Library

### Message queue

Message queue is map accessed by the device address.

Each element is an vector or map accessed by message sequence number.

Each element consist of

- message sequence number (if container is vector)
- network server metadata such as receiving time
- radio packet itself
- metadata sent by the gateway- map of gateway identifier

Each element of the message queue has an associated task descriptor.

There is 4 operations on the queue:

- add a new message to the queue
- iterate messages in the queue - get messge fron the queue one by one
- update message in the queue, message accessed by the address and the sequence number. It updates task descriptor stage
- remove cancelled or completed tasks from the queue

Receiver object adds a new messages to the queue, receive network and app keys over program interfaces and update received keys in the task descriptor.

If receiver successfully receives key for address, it access message in the queue by the address and set key in the task descriptor. 
Then it increment task stage to the "got key"

Sender object iterates messages in the queue with "got key" and initiate decipher packet. Decipher can be run in the main thread or runt in another thread, when it done, the Receiver must be receive result.

Sender looks what stage is and initiates tasks to obtain keys by initiaite sending requiests.

Sender initiate send response to the gateway. When response is successfully sent or sent with errors task descriptor must indicates response success or failure. 

MAC processor can create a message to be send to the end-device over best gateway.

### Task descriptor

Task has stages:

- just received
- merged or unique. If packet received by two or more gateways, identical messages merged into the one. Metadata specific to the gateway added to the array of metadatas.
- got network key
- got app key
- deciphered
- MAC command process initiated
- accepted or declined (sent to app server or to error log)

After message is in the last stage, message deleted from the queue.

Task descriptor consists of

- stage
- network key
- app key
- stage process error code

At any stage task can be cancelled on stage process error such as
- no network or app key available (device is not registered)
- CRC error after decipher
- queue is full (no memory to process new message)

### Scheduler

Task scheduler 

### Receiver

Receiver object read

- packets from gateways and try to parse received packet
- network and app keys from

from the socket or file and then update message stage in the queue.

Receiver wait until socket or file descriptor indicates data arrived using select() call.

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

Message queue is a main structure.

Message queue object provide

- put accepted message (used by Receiver)
- list of messages need to reply to the gateway
- list of messages ready to send to the app server
- list of messages 

### Main event loop procedure

```
loop
  wait for event or timeout
  if timeout
  then run sheduler
  else Receiver.receive
end loop
```
