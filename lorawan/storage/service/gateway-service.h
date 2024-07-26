#ifndef GATEWAY_SERVICE_H_
#define GATEWAY_SERVICE_H_ 1

#include <map>
#include <vector>

#include "lorawan/lorawan-types.h"
#include "lorawan/storage/gateway-identity.h"

/**
 * Identity service interface
 * Get device identifier and keys by the network address
 */ 
class GatewayService {
protected:
public:
    GatewayService();
    virtual ~GatewayService();
    /**
    * request device identifier(w/o address) by network address. Return 0 if success, retVal = EUI and keys
    * @param retVal return gateway address or identifier
    * @param request gateway identifier and last address
    * @return CODE_OK- success
    */
    virtual int get(GatewayIdentity &retVal, const GatewayIdentity &request) = 0;

    // Add or replace Address = EUI and keys pair
    virtual int put(const GatewayIdentity &identity) = 0;

    // Remove entry
    virtual int rm(const GatewayIdentity &identity) = 0;

    /**
     * List entries
     * @param retVal return values
     * @param offset 0..
     * @param size 0- all
     * @return CODE_OK- success
     */

    virtual int list(
        std::vector<GatewayIdentity> &retVal,
        uint32_t offset,
        uint8_t size
    ) = 0;

    // Entries count
    virtual size_t size() = 0;

    // force save
    virtual void flush() = 0;

    // reload
    virtual int init(const std::string &option, void *data) = 0;

    // close resources
    virtual void done() = 0;

    virtual void setOption(int option, void *value) = 0;
};

#endif
