#include "lorawan/lorawan-error.h"
#include "lorawan/storage/service/device-best-gateway.h"

DeviceBestGatewayService::DeviceBestGatewayService(
    DeviceBestGatewayResponseClient *aClient
)
    : client(aClient)
{
}

/**
 * asynchronous request best gateway for device by address
 * Return best gateway identifier in callback
 * @param devAddr network address
 * @return CODE_OK success, <0- error code
 */
int DeviceBestGatewayService::cGet(
        const DEVADDR &devAddr
) {
    if (client) {
        auto r = get(devAddr);
        client->onDeviceBestGatewayGet(this->client, devAddr, r);
        return CODE_OK;
    }
    return ERR_CODE_PARAM_INVALID;
};

/**
 * asynchronous add or replace Address => gw
 * Optional method
 * @param devAddr network address
 * @param id network identity
 * @return CODE_OK- success
 */
int DeviceBestGatewayService::cPut(
    const DEVADDR &devAddr,
    uint64_t gwId
) {
    if (client) {
        int r = put(devAddr, gwId);
        client->onDeviceBestGatewayOperation(this->client, devAddr, r);
        return CODE_OK;
    }
    return ERR_CODE_PARAM_INVALID;
}

/**
* asynchronous remove entry
* @param devAddr address to remove
* @return CODE_OK- success
*/
int DeviceBestGatewayService::cRm(
    const DEVADDR &devAddr
) {
    if (client) {
        int r = rm(devAddr);
        client->onDeviceBestGatewayOperation(this->client, devAddr, r);
        return CODE_OK;
    }
    return ERR_CODE_PARAM_INVALID;
}

/**
 * asynchronous list entries
 * @param offset 0..
 * @param size 0- all
 */
int DeviceBestGatewayService::cList(
    uint32_t offset,
    uint8_t size
) {
    if (client) {
        std::map<DEVADDR, uint64_t > m;
        int r = list(m, offset, size);
        client->onDeviceBestGatewayList(this->client, m);
        return CODE_OK;
    }
    return ERR_CODE_PARAM_INVALID;
}

/**
 * synchronous entries count
 * @return CODE_OK success
 */
int DeviceBestGatewayService::cSize()
{
    if (client) {
        size_t r = size();
        client->onDeviceBestGatewaySize(this->client, r);
        return CODE_OK;
    }
    return ERR_CODE_PARAM_INVALID;

}

