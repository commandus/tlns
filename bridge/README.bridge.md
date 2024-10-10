# Specific application bridges

This folder contains application bridge examples for well-known protocols such MQTT.

You need install third-party dependencies to build up this examples.

When Dispatcher receives message it send payload to the application bridge.

To use application bridge you need

- Instantiate AppBridge object
- Pass AppBridge object to the dispatcher

You can see bridge examples in

- examples in the lorawan/bridge folder (stdout-bridge, file-json-bridge)
- specific examples in the bridge/ folder (mqtt/bridge-mqtt-wss)


## Instantiate AppBridge object

Create inherited class override AppBridge's methods:

- onPayload(),
- init(),
- done()

and then create an object of new class

init() method reserve 2 string option and one void pointer to any data which you can use to initialize bridge.

## Pass bridge to the dispatcher

Dispatcher (MessageTaskDispatcher class) has a method addAppBridge() to add one or more application bridges.

Pass to addAppBridge() method pointer to AppBridge object.

After dispatcher has been destroyed delete AppBridge object.

MessageTaskDispatcher class keep collection of bridges in the PluginBridges class. 
