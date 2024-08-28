#include "lorawan/proto/gw/json-wired-client.h"

#include <lorawan/lorawan-error.h>

JsonWiredClient::JsonWiredClient(
    const DirectClient *aDirectClient,
    uint64_t aGwId,
    const std::string &aNetworkServerAddress,
    uint16_t aNetworkServerPort,
    const DEVADDR &aDeviceAddress
)
    : directClient(aDirectClient), gwId(aGwId), networkServerAddress(aNetworkServerAddress),
    networkServerPort(aNetworkServerPort), deviceAddress(aDeviceAddress), running(false)
{
}

JsonWiredClient::~JsonWiredClient() = default;

int JsonWiredClient::JsonWiredClient::send(
    const std::string &fopts,
    const std::string &payload
)
{
    return CODE_OK;
}

int JsonWiredClient::JsonWiredClient::run() {
    running = true;
    if (openConnection() != CODE_OK)
        return ERR_CODE_SOCKET_OPEN;
    while(running) {

    }
    closeConnection();
    return CODE_OK;
}

void JsonWiredClient::JsonWiredClient::stop()
{
    running = false;
}

int JsonWiredClient::openConnection() {
    return CODE_OK;
}

void JsonWiredClient::closeConnection() {
}