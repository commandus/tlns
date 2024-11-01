# Gateway protocols

Network server communicate with end-device over gateway.

Dispatcher receive message from the gateway and try to decode it using one of known protocols such
LNS (Basic communication protocol between Lora gateway and server) by the Semtech (Cycleo)
(see [lorawan/proto/gw/basic-usp](lorawan/proto/gw/basic-usp)).

Anyhow gateway can use other protocol.

To use your own protocol you need

- Instantiate ProtoGwParser object (lorawan/proto/gw/proto-gw-parser files)
- Pass ProtoGwParser object to the dispatcher (dispatcher must have one or more protocols)

Dispatcher try parse received packet one by one each protocol until parse() method was successful.

You can see bridge examples in the lorawan/proto/gw/ folder (json-wired)

## Instantiate ProtoGwParser object

Create inherited class override ProtoGwParser's methods:

- name()
- tag()
- parse()
- ack()
- makeMessage2Gateway()

and then create an object of new class

name() method return protocol name, tag() return unique protocol number.

parse() parse received packet, ack() construct ACK packet etc. 

## Pass bridge to the dispatcher

Dispatcher (MessageTaskDispatcher class) has a method addParser() to add one or more gateway protocol.

When dispatcher receive an packet it 
