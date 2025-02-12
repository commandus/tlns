#ifndef ASYNC_WRAPPER_IDENTITY_SERVICE_H_
#define ASYNC_WRAPPER_IDENTITY_SERVICE_H_ 1

#include <functional>
#include "identity-service.h"

/**
 * Identity service async wrapper
 */
class AsyncWrapperIdentityService {
private:
    IdentityService *identityService;
public:
    explicit AsyncWrapperIdentityService(IdentityService *value);

    void get(
        const DEVADDR &devAddr,
        const std::function<void(
            int retCode,
            DEVICEID &retVal
        )>& cb
    );

    void getNetworkIdentity(
        const DEVEUI &eui,
        const std::function<void(
            int retCode,
            NETWORKIDENTITY &retVal
        )>& cb
    );

    // Add or replace Address = EUI and keys pair
    void put(
        const DEVADDR &devaddr,
        const DEVICEID &id,
        const std::function<void(
            int retCode
        )>& cb
    );

    // Remove
    void rm(
        const DEVADDR &addr,
        const std::function<void(
            int retCode
        )>& cb
    );

    void list(
        uint32_t offset,
        uint8_t size,
        const std::function<void(
            int retCode,
            std::vector<NETWORKIDENTITY> &retval
        )>& cb
    );

    // Entries count
    void size(
        const std::function<void(
            size_t size
        )>& cb
    );

    // force save
    void flush(
        const std::function<void(
            int retCode
        )>& cb
    );

    // reload
    void init(
        const std::string &option,
        void *data,
        const std::function<void(
            int retCode
        )>& cb
    );

    // close resources
    void done(
        const std::function<void(
            int retCode
        )>& cb
    );

    void next(
        const std::function<void(
            NETWORKIDENTITY &retVal
        )>& cb
    );

    void setOption(
        int option,
        void *value,
        const std::function<void(
            int retCode
        )>& cb
    );

};

#endif
