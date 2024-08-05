#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

#include "bridge/mqtt/bridge-mqtt-wss.h"
#include "lorawan/helper/tlns-cli-helper.h"

// Assume a local server with websocket support on port 8080
static mqtt::connect_options makeWSSClientConnectionOptions(
    const std::string &proxyUrl,
    const std::string &user,
    const std::string &password
)
{
    auto connBuilder = mqtt::connect_options_builder::ws();
    if (!proxyUrl.empty())
        connBuilder.http_proxy(proxyUrl);
    auto sslopts = mqtt::ssl_options_builder()
        .error_handler([](const std::string& msg) {
            std::cerr << "SSL Error: " << msg << std::endl;
        })
        .enable_server_cert_auth(false)
        .ssl_version(MQTT_SSL_VERSION_TLS_1_1)
        .finalize();
    auto connOpts = connBuilder
        .keep_alive_interval(std::chrono::seconds(45))
        .user_name(user)
        .password(password)
        .ssl(std::move(sslopts))
        .finalize();
    return std::move(connOpts);
}

static int sendWSSSmth(
    mqtt::async_client *client,
    int qos,
    const std::string &topic,
    const std::string &msgText
) {
    bool ok = false;
    try {
        auto msg = mqtt::make_message(topic, msgText, 1, false);
        if (client)
            ok = client->publish(msg)->wait_for(std::chrono::seconds(2));
    } catch (const mqtt::exception& exc) {
        std::cerr << exc.get_error_str() << std::endl;
        return -3;
    }
    return ok ? 0 : -4;
}

void MqttWssBridge::onPayload(
    const void* dispatcher,   // MessageTaskDispatcher*
    const MessageQueueItem *messageItem, // network identity, gateway identifier and metadata etc.
    const char *value,
    size_t size
)
{
    if (messageItem) {
        errorCode = sendWSSSmth(cli, 1, topic, messageItem->toJsonString(value, size));
    }
}

MqttWssBridge::MqttWssBridge()
    : errorCode(0)
{
}

void MqttWssBridge::init(
    const std::string& aWssUrl,
    const std::string& aProxyUrl,
    const void *userPasswordTopic
)
{
    url = aWssUrl;
    proxyUrl = aProxyUrl;
    if (userPasswordTopic) {
        splitUserPasswordTopic(user, password, topic, *(std::string *) userPasswordTopic);
    }

    auto connOpts = makeWSSClientConnectionOptions(proxyUrl, user, password);
    cli = new mqtt::async_client(url, "tlns-bridge-mqtt-wss-client");
    try {
        // Connect to the server
        cli->connect(connOpts)->wait();
    } catch (const mqtt::exception& exc) {
        std::cerr << exc.get_error_str() << std::endl;
        errorCode = -1;
    }
    errorCode = 0;
}

void MqttWssBridge::done()
{
    try {
        cli->disconnect()->wait();
    } catch (const mqtt::exception& exc) {
        std::cerr << exc.get_error_str() << std::endl;
        errorCode = -2;
    }
}


EXPORT_SHARED_C_FUNC AppBridge* makeStdoutBridge()
{
    return new MqttWssBridge;
}
