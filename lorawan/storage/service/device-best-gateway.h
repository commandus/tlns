#ifndef DEVICE_BEST_GATEWAY_H_
#define DEVICE_BEST_GATEWAY_H_ 1

#include <map>
#include <vector>

#include "lorawan/lorawan-types.h"
#include "lorawan/storage/client/response-device-best-gateway-client.h"

/**
 * Device address to gateway id service interface
 * Get device identifier and keys by the network address
 * If responseClient is provided, it is asynchronous service
 * Use appropriate methods:
 *
 * Synchronous version                        Asynchronous version
 * -------------------                        --------------------
 * getUplink(const DEVADDR &devAddr)                cGet(const DEVADDR &devAddr)
 * putUplink(const DEVADDR &devAddr, uint64_t gwId) cPut(const DEVADDR &devAddr, uint64_t gwId)
 * rm(const DEVADDR &addr)                    cRm(const DEVADDR &addr)
 * ...
 */
class DeviceBestGatewayService {
protected:
    DeviceBestGatewayResponseClient *client;
public:
    explicit DeviceBestGatewayService(
        DeviceBestGatewayResponseClient *client = nullptr
    );

    /**
    * synchronous request best gateway for device by address
    * @param retVal device identifier
    * @return best gateway identifier. 0- error occurred
    */

    virtual uint64_t get(
        const DEVADDR &devAddr
    ) = 0;

    /**
     * asynchronous request best gateway for device by address
     * Return best gateway identifier in callback
     * @param devAddr network address
     * @return CODE_OK success, <0- error code
     */
    int cGet(
        const DEVADDR &devAddr
    );

    /**
     * synchronous add or replace Address => gw
     * Optional method
     * @param devaddr network address
     * @param id network identity
     * @return CODE_OK- success
     */
    virtual int put(
        const DEVADDR &devaddr,
        uint64_t gwId
    ) = 0;

    /**
     * asynchronous add or replace Address => gw
     * Optional method
     * @param devaddr network address
     * @param id network identity
     * @return CODE_OK- success
     */
    int cPut(
        const DEVADDR &devaddr,
        uint64_t gwId
    );

    /**
     * synchronous remove entry
     * Optional method
     * @param addr address to remove
     * @return CODE_OK- success
     */
    virtual int rm(
        const DEVADDR &addr
    ) = 0;

    /**
     * asynchronous remove entry
     * @param addr address to remove
     * @return CODE_OK- success
     */
    int cRm(
        const DEVADDR &addr
    );

    /**
     * synchronous list entries
     * @param retVal return values
     * @param offset 0..
     * @param size 0- all
     */
    virtual int list(
        std::map<DEVADDR, uint64_t> &retVal,
        uint32_t offset,
        uint8_t size
    ) = 0;

    /**
     * asynchronous list entries
     * @param offset 0..
     * @param size 0- all
     */
    int cList(
        uint32_t offset,
        uint8_t size
    );

    /**
     * synchronous entries count
     * @return
     */
    virtual size_t size() = 0;

    /**
     * synchronous entries count
     * @return CODE_OK success
     */
    int cSize();

    /**
     * synchronous force save
     */
    virtual void flush() = 0;

    /**
     * Initialize storage
     * @param option
     * @param data
     * @return
     */
    virtual int init(
        const std::string &option,
        void *data
    ) = 0;

    /**
     * Finalize. Close resources
     */
    virtual void done() = 0;

    /**
     * Set options
     * @param option 0- masterkey 1- code 2- accesscode
     * @param value 0- string 1- int32_t 2- uint64_t
     */
    virtual void setOption(
        int option,
        void *value
    ) = 0;
};

#endif
