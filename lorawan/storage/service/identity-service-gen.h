#ifndef IDENTITY_SERVICE_GEN_H_
#define IDENTITY_SERVICE_GEN_H_ 1

#include <vector>
#include <mutex>
#include <map>
#include "lorawan/storage/service/identity-service.h"

#include "lorawan/helper/plugin-helper.h"

class GenIdentityService: public IdentityService {
private:
    NETID netid;
    KEY128 key;
    // helper data
    // helps to find out free address in the space
    uint32_t maxDevNwkAddr;
protected:
    std::string masterKey;
    void clear();
    /**
      * Return next network address if available
      * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
      */
    int nextBruteForce(NETWORKIDENTITY &retVal);
    void gen(NETWORKIDENTITY &retVal, uint32_t nwkAddr);
public:
    int errCode;
    std::string errMessage;

    GenIdentityService();
    GenIdentityService(const std::string &masterKey);
    ~GenIdentityService() override;
    void setMasterKey(const std::string &masterKey);

    // synchronous calls
    int get(DEVICEID &retval, const DEVADDR &devaddr) override;
    int getNetworkIdentity(NETWORKIDENTITY &retval, const DEVEUI &eui) override;
    int put(const DEVADDR &devaddr, const DEVICEID &id) override;
    int rm(const DEVADDR &addr) override;
    int list(std::vector<NETWORKIDENTITY> &retVal, uint32_t offset, uint8_t size) override;
    size_t size() override;
    int next(NETWORKIDENTITY &retVal) override;

    // asynchronous imitation
    int cGet(const DEVADDR &request) override;
    int cGetNetworkIdentity(const DEVEUI &eui) override;
    int cPut(const DEVADDR &devAddr, const DEVICEID &id) override;
    int cRm(const DEVADDR &devAddr) override;
    int cList(uint32_t, uint8_t size) override;
    int cSize() override;
    int cNext() override;

    int filter(
        std::vector<NETWORKIDENTITY> &retVal,
        const std::vector<NETWORK_IDENTITY_FILTER> &filters,
        uint32_t offset,
        uint8_t size
    ) override;
    int cFilter(
        const std::vector<NETWORK_IDENTITY_FILTER> &filters,
        uint32_t offset,
        uint8_t size
    ) override;

    int init(const std::string &option, void *data) override;
    void flush() override;
    void done() override;
    /**
      * Return next network address if available
      * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
      */
    void setOption(int option, void *value) override;
};

EXPORT_SHARED_C_FUNC IdentityService * makeIdentityService();

#endif
