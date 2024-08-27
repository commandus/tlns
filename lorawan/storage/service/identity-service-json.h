#ifndef IDENTITY_SERVICE_JSON_H_
#define IDENTITY_SERVICE_JSON_H_ 1

#include "third-party/nlohmann/json.hpp"
#include "lorawan/storage/service/identity-service-mem.h"
#include "platform-specific.h"

class JsonIdentityService: public MemoryIdentityService {
private:
    bool load();
    bool store();
protected:
    std::string fileName;
public:
    JsonIdentityService();
    ~JsonIdentityService() override;
    int get(DEVICEID &retVal, const DEVADDR &request) override;
    // List entries
    int list(std::vector<NETWORKIDENTITY> &retVal, uint32_t offset, uint8_t size) override;
    size_t size() override;
    int getNetworkIdentity(NETWORKIDENTITY &retVal, const DEVEUI &eui) override;

    int put(const DEVADDR &devAddr, const DEVICEID &id) override;
    int rm(const DEVADDR &addr) override;

    int init(const std::string &dbName, void *db) override;
    void flush() override;
    void done() override;

    /**
     * Return next network address if available
     * @return 0- success, ERR_ADDR_SPACE_FULL- no address available
     */
    int next(NETWORKIDENTITY &retVal) override;
    void setOption(int option, void *value) override;
};

EXPORT_SHARED_C_FUNC IdentityService* makeIdentityService1();

#endif
