#ifndef IDENTITY_SERVICE_H_
#define IDENTITY_SERVICE_H_ 1

#include <map>
#include <vector>

#include "lorawan/lorawan-types.h"
#include "lorawan/storage/client/response-client.h"

/**
 * Identity service interface
 * Get device identifier and keys by the network address
 * If responseClient is provided, it is asynchronous service
 * Use appropriate methods:
 *
 * Synchronous version                                                    Asynchronous version
 *
 * get(DEVICEID &retVal, const DEVADDR &devAddr)                          cGet(const DEVADDR &devAddr)
 * getNetworkIdentity(NETWORKIDENTITY &retval, const DEVEUI &eui)         cGetNetworkIdentity(const DEVEUI &eui)
 * put(const DEVADDR &devaddr, const DEVICEID &id)                        cPut(const DEVADDR &devaddr, const DEVICEID &id)
 * rm(const DEVADDR &addr)                                                cRm(const DEVADDR &addr)
 * list(std::vector<NETWORKIDENTITY> &retVal, size_t offset, size_t size) cList(size_t offset, size_t size)
 * filter(std::vector<NETWORKIDENTITY> &retVal, const std::vector<NETWORK_IDENTITY_FILTER> &filters, size_t offset, size_t size)
 *  cFilter(const std::vector<NETWORK_IDENTITY_FILTER> &filters, size_t offset, size_t size)
 * size()                                                                 cSize()
 * next(NETWORKIDENTITY &retVal                                           cNext()
 */
class IdentityService {
protected:
    // LoraWAN network identifier
    NETID netid;
    /**
     * If responseClient is NULL, service is synchronous, otherwise is asynchronous
     */
    ResponseClient *responseClient;
public:
    IdentityService();
    /**
     * Constructor for asynchronous service
     * @param responseClient
     */
    IdentityService(ResponseClient *responseClient);
    /**
     * Constructor for synchronous service
     * @param value
     */
    IdentityService(const IdentityService &value);
    virtual ~IdentityService();

    /**
    * synchronous request device identifier(w/o address) by network address. Return 0 if success, retVal = EUI and keys
    * @param retVal device identifier
    * @param devAddr network address
    * @return LORA_OK- success
    */
    virtual int get(DEVICEID &retVal, const DEVADDR &devAddr) = 0;
    /**
     * asynchronous request device identifier(w/o address) by network address
     * Return device identifier in responseClient callback
     * @param devAddr network address
     * @return CODE_OK.
     */
    virtual int cGet(const DEVADDR &devAddr) = 0;

    /**
    * synchronous request network identity(with address) by network address. Return 0 if success, retval = EUI and keys
    * @param retval network identity(with address)
    * @param eui device EUI
    * @return CODE_OK- success
    */
    virtual int getNetworkIdentity(NETWORKIDENTITY &retval, const DEVEUI &eui) = 0;
    /**
     * asynchronous request network identity(with address) by network address.
     * Return network identity(with address in responseClient callback
     * @param eui device EUI
     * @return CODE_OK- success
     */
    virtual int cGetNetworkIdentity(const DEVEUI &eui) = 0;

    /**
     * synchronous add or replace Address = EUI and keys pair
     * @param devaddr network address
     * @param id network identity
     * @return CODE_OK- success
     */
    virtual int put(const DEVADDR &devaddr, const DEVICEID &id) = 0;
    /**
     * asynchronous add or replace Address = EUI and keys pair
     * @param devaddr network address
     * @param id network identity
     * @return CODE_OK- success
     */
    virtual int cPut(const DEVADDR &devaddr, const DEVICEID &id) = 0;

    /**
     * synchronous remove entry
     * @param addr address to remove
     * @return CODE_OK- success
     */
    virtual int rm(const DEVADDR &addr) = 0;
    /**
     * asynchronous remove entry
     * @param addr address to remove
     * @return CODE_OK- success
     */
    virtual int cRm(const DEVADDR &addr) = 0;

    /**
     * synchronous list entries
     * @param retVal return values
     * @param offset 0..
     * @param size 0- all
     * @return 0- success
     */
    virtual int list(
        std::vector<NETWORKIDENTITY> &retVal,
        uint32_t offset,
        uint8_t size
    ) = 0;

    /**
     * synchronous list entries with filter(s)
     * @param retVal return values
     * @param compareWith identity to compare
     * @param filters filters
     * @param offset 0..
     * @param size 0- all
     * @return 0- success
     */
    virtual int filter(
        std::vector<NETWORKIDENTITY> &retVal,
        const std::vector<NETWORK_IDENTITY_FILTER> &filters,
        uint32_t offset,
        uint8_t size
    ) = 0;

    /**
     * asynchronous list entries
     * @param offset 0..
     * @param size 0- all
     */
    virtual int cList(
        uint32_t offset,
        uint8_t size
    ) = 0;

    /**
     * asynchronous list entries with filter(s)
     * @param compareWith identity to compare
     * @param filters filters
     * @param offset 0..
     * @param size 0- all
     */
    virtual int cFilter(
        const std::vector<NETWORK_IDENTITY_FILTER> &filters,
        uint32_t offset,
        uint8_t size
    ) = 0;

    /**
     * synchronous entries count
     * @return
     */
    virtual size_t size() = 0;
    /**
     * synchronous entries count
     * @return
     */
    virtual int cSize() = 0;

    /**
     * synchronous call.
     * Return next network address if available
     * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
     */
    virtual int next(
            NETWORKIDENTITY &retVal
    ) = 0;

    /**
     * asynchronous call.
     * Return next network address if available in callback
     * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
     */
    virtual int cNext() = 0;

    /**
     * synchronous force save
     */
    virtual void flush() = 0;

    /**
     * Initialize
     * @param option
     * @param data
     * @return
     */
    virtual int init(const std::string &option, void *data) = 0;

    /**
     * Finalize. Close resources
     */
    virtual void done() = 0;

    /**
     * Set options
     * @param option 0- masterkey 1- code 2- accesscode
     * @param value 0- string 1- int32_t 2- uint64_t
     */
    virtual void setOption(int option, void *value) = 0;

    virtual NETID *getNetworkId();

    virtual void setNetworkId(
        const NETID &value
    );

    int joinAccept(
        JOIN_ACCEPT_FRAME_HEADER &retVal,
        NETWORKIDENTITY &networkIdentity
    );
};

#endif
