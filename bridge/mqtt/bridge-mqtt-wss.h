#ifndef TLNS_MQTT_WSS_BRIDGE_H
#define TLNS_MQTT_WSS_BRIDGE_H

#include "lorawan/bridge/app-bridge.h"
#include "mqtt/async_client.h"

class MqttWssBridge : public AppBridge {
protected:
    mqtt::async_client *cli;
    int errorCode;
    // e.g. "wss://mqtt.commandus.com:443"
    std::string url;
    // A local proxy, like squid on port 3128 basic authentication with user "user" and password "pass" e.g. "http://user:pass@localhost:3128"
    const std::string DFLT_PROXY_ADDRESS();
    std::string proxyUrl;
    // e.g. "tlns"
    std::string user;
    std::string password;
    std::string topic;
public:
    MqttWssBridge();
    virtual ~MqttWssBridge() = default;
    void onPayload(
        const void* dispatcher,                 // MessageTaskDispatcher*
        const MessageQueueItem *messageItem     // network identity, gateway identifier and metadata etc.
    ) override;
    void init(
        const std::string& wssUrl,
        const std::string& proxyUrl,
        const void *userPasswordTopic
    ) override;
    void done() override;
};

EXPORT_SHARED_C_FUNC AppBridge* makeMqttWssBridge();

#endif
