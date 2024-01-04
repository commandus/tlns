# tlns

Tiny LoRaWAN Network Server

The goal of the project is to make a network server running on the ESP32 board.

The working release of [https://github.com/commandus/lorawan-network-server](https://github.com/commandus/lorawan-network-server) runs on Raspberry. This requires much more resources.

Network server can serve 1, 2 or more gateways.

If there 2 or more gateways, message can recieved twice or more. 

Network server collects metadata sent by gateways to choose gateway with the best signal/noise ratio to send response. 

## Tools

- Visual Code
  - Draw.io Integration Visual Code plugin
  - For ESP32 install Visual Code "Espressif IDF" plugin and configure ESP-IDF 

## Library

Library operates with two high-level class of objects:

- Messages stored in the message queue
- Tasks

Task is a glue to make asynchronius requests.
Task associated with the message in the message queue.
Task contains process stage and last operation result and intermediate data such as keys to decipher payload.

Two object classes

- Dispatcher
- Services

operates with tasks. For instance, the "Receiver" service create a new task and move it to the dispatcher.

Then Dispatcher put task to the "Get device identity" service. 
"Get device identity" service after receiving keys return keys back tro the Dispatcher.

Dispatcher get one task from the queue and move it to the appropriate service. 

Service must put task in internal queue. When task is complete, service move task to the dispacher queue.

### Message queue

Message queue is map accessed by the device address.

Each element is an vector or map accessed by message sequence number.

Each element consist of

- message sequence number (if container is a vector)
- receiving time of the first received packet (no matter which gateway is first)
- radio packet itself
- metadata sent by the gateway. Metadata stored in the map of gateway identifier

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
- got device identifier: network/app keys
- deciphered
- MAC command process initiated
- accepted or declined (sent to app server or to error log)

After message is in the last stage, message waits to expire and after expiration time is deleted from the queue.

Task descriptor consists of

- stage
- device identifer: network key/app key
- stage process error code
- radio metadata array sent by the gateway such as signal power, signal/noise ratio etc.

If packet received by two or more gateways, identical messages merged into the one. Metadata specific to the gateway added to the array of metadatas.
Radio metadata array have at least 1 element.

At any stage task can be cancelled on stage process error such as
- no network or app key available (device is not registered)
- CRC error after decipher
- queue is full (no memory to process new message)

### Dispatcher

Dispatcher serves message queue. 
Each time dispatcher has been called it get one or more task descriptor ready to serve.

Then dispatcher start one or more tasks:

- send keys requests
- decipher message
- collect radio statistics and select best gateway for each end-device
- send reply to the best gateway  
- send MAC commands to the end-device
- send messages initiated by the app server

### Receiver

Receiver object read

- packets from gateways and try to parse received packet
- network and app keys from the socket or file and then update message stage in the queue.
- end-device recomendation what gateway it the best for end-device, last received sequence number
- receives message to be send to the end-device from the app server

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
