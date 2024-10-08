#include "lorawan/lorawan-error.h"
#include "lorawan/storage/service/device-best-gateway-mem.h"

/**
 * Device address to gateway id service interface
 * Get device identifier and keys by the network address
 * If responseClient is provided, it is asynchronous service
 * Use appropriate methods:
 *
 * Synchronous version                        Asynchronous version
 * -------------------                        --------------------
 * get(const DEVADDR &devAddr)                cGet(const DEVADDR &devAddr)
 * put(const DEVADDR &devAddr, uint64_t gwId) cPut(const DEVADDR &devAddr, uint64_t gwId)
 * rm(const DEVADDR &addr)                    cRm(const DEVADDR &addr)
 * ...
 */
DeviceBestGatewayServiceMem::DeviceBestGatewayServiceMem(
    size_t aLimit,
    DeviceBestGatewayResponseClient *client
)
    : DeviceBestGatewayService(client), limit(aLimit)
{

}

/**
* synchronous request best gateway for device by address
* @param retVal device identifier
* @return best gateway identifier. 0- error occurred
*/
uint64_t DeviceBestGatewayServiceMem::get(
    const DEVADDR &devAddr
) {
    auto it = storage.find(devAddr);
    if (it == storage.end())
        return 0;
    return it->second;
}

/**
 * synchronous add or replace Address => gw
 * Optional method
 * @param devaddr network address
 * @param id network identity
 * @return CODE_OK- success
 */
int DeviceBestGatewayServiceMem::put(
    const DEVADDR &devaddr,
    uint64_t gwId
) {
    storage[devaddr] = gwId;
    return CODE_OK;
}

/**
 * synchronous remove entry
 * Optional method
 * @param addr address to remove
 * @return CODE_OK- success
 */
int DeviceBestGatewayServiceMem::rm(
    const DEVADDR &devAddr
) {
    auto it = storage.find(devAddr);
    if (it == storage.end())
        return ERR_CODE_INVALID_ADDRESS;
    storage.erase(it);
    return CODE_OK;
}

/**
 * synchronous list entries
 * @param retVal return values
 * @param offset 0..
 * @param size 0- all
 */
int DeviceBestGatewayServiceMem::list(
    std::map<DEVADDR, uint64_t> &retVal,
    uint32_t offset,
    uint8_t size
) {
    uint32_t o = 0;
    uint32_t c = 0;
    for (auto &it : storage) {
        if (o < offset) {
            o++;
            continue;
        }
        c++;
        if (c > size)
            break;
        retVal[it.first] = it.second;
    }
    return CODE_OK;
}

/**
 * synchronous entries count
 * @return
 */
size_t DeviceBestGatewayServiceMem::size() {
    return storage.size();
}

/**
 * synchronous force save
 */
void DeviceBestGatewayServiceMem::flush() {
}

/**
 * Initialize storage
 * @param option
 * @param data
 * @return
 */
int DeviceBestGatewayServiceMem::init(
    const std::string &option,
    void *data
) {
    return CODE_OK;
}

/**
 * Finalize. Close resources
 */
void DeviceBestGatewayServiceMem::done() {
}

/**
 * Set options
 * @param option 0- masterkey 1- code 2- accesscode
 * @param value 0- string 1- int32_t 2- uint64_t
 */
void DeviceBestGatewayServiceMem::setOption(
    int option,
    void *value
) {

}
