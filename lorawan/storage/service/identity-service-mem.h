#ifndef IDENTITY_SERVICE_MEM_H_
#define IDENTITY_SERVICE_MEM_H_ 1

#include "lorawan/storage/service/identity-service.h"
#include "lorawan/helper/plugin-helper.h"

class MemoryIdentityService: public IdentityService {
protected:
    std::map<DEVADDR, DEVICEID> storage;
public:
    MemoryIdentityService();
    ~MemoryIdentityService() override;

    // synchronous
    int get(DEVICEID &retVal, const DEVADDR &request) override;
    int getNetworkIdentity(NETWORKIDENTITY &retVal, const DEVEUI &eui) override;
    int put(const DEVADDR &devAddr, const DEVICEID &id) override;
    int rm(const DEVADDR &devAddr) override;
    int list(std::vector<NETWORKIDENTITY> &retVal, uint32_t offset, uint8_t size) override;
    size_t size() override;
    int next(NETWORKIDENTITY &retVal) override;
    // asynchronous imitation
    int cGet(const DEVADDR &request) override;
    int cGetNetworkIdentity(const DEVEUI &eui) override;
    int cPut(const DEVADDR &devAddr, const DEVICEID &id) override;
    int cRm(const DEVADDR &devAddr) override;
    int cList(uint32_t offset, uint8_t size) override;
    int cSize() override;
    int cNext() override;

    int init(const std::string &dbName, void *db) override;
    void flush() override;
    void done() override;
    void setOption(int option, void *value) override;
};

EXPORT_SHARED_C_FUNC IdentityService* makeIdentityService2();

#endif
