#ifndef DEVICE_BEST_GATEWAY_MEM_H_
#define DEVICE_BEST_GATEWAY_MEM_H_ 1

#include "lorawan/storage/service/device-best-gateway.h"

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
class DeviceBestGatewayServiceMem : public DeviceBestGatewayService {
private:
    std::map <DEVADDR, uint64_t > storage;
protected:
    size_t limit;
public:
    explicit DeviceBestGatewayServiceMem(
        size_t limit = 0,
        DeviceBestGatewayResponseClient *client = nullptr
    );

    /**
    * synchronous request best gateway for device by address
    * @param retVal device identifier
    * @return best gateway identifier. 0- error occurred
    */
    uint64_t get(
        const DEVADDR &devAddr
    ) override;

    /**
     * synchronous add or replace Address => gw
     * Optional method
     * @param devaddr network address
     * @param id network identity
     * @return CODE_OK- success
     */
    int put(
        const DEVADDR &devaddr,
        uint64_t gwId
    ) override;

    /**
     * synchronous remove entry
     * Optional method
     * @param addr address to remove
     * @return CODE_OK- success
     */
    int rm(
        const DEVADDR &addr
    ) override;

    /**
     * synchronous list entries
     * @param retVal return values
     * @param offset 0..
     * @param size 0- all
     */
    int list(
        std::map<DEVADDR, uint64_t> &retVal,
        uint32_t offset,
        uint8_t size
    ) override;

    /**
     * synchronous entries count
     * @return
     */
    size_t size() override;

    /**
     * synchronous force save
     */
    void flush() override;

    /**
     * Initialize storage
     * @param option
     * @param data
     * @return
     */
    int init(
        const std::string &option,
        void *data
    ) override;

    /**
     * Finalize. Close resources
     */
    void done() override;

    /**
     * Set options
     * @param option 0- masterkey 1- code 2- accesscode
     * @param value 0- string 1- int32_t 2- uint64_t
     */
    void setOption(
        int option,
        void *value
    ) override;
};

#endif
