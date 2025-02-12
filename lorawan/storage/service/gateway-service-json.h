#ifndef GATEWAY_SERVICE_JSON_H_
#define GATEWAY_SERVICE_JSON_H_ 1

#include <vector>
#include <mutex>
#include <map>
#include "lorawan/storage/service/gateway-service-mem.h"
#include "lorawan/helper/plugin-helper.h"

class JsonGatewayService: public MemoryGatewayService {
private:
    bool load();
    bool store();
protected:
    std::string fileName;
public:
    JsonGatewayService();
    ~JsonGatewayService() override;
    int get(GatewayIdentity &retVal, const GatewayIdentity &request) override;
    // List entries
    int list(std::vector<GatewayIdentity> &retVal,
        uint32_t offset,
        uint8_t size
    ) override;
    // Entries count
    size_t size() override;
    int put(const GatewayIdentity &request) override;
    int rm(const GatewayIdentity &addr) override;

    int init(const std::string &option, void *data) override;
    void flush() override;
    void done() override;
    void setOption(int option, void *value) override;
};

EXPORT_SHARED_C_FUNC GatewayService* makeGatewayService1();

#endif
