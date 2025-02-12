#ifndef UV_LISTENER_H_
#define UV_LISTENER_H_	1

#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#include <string>
#if defined(_MSC_VER) || defined(__MINGW32__)
#else
#include <arpa/inet.h>
#endif


#include "storage-listener.h"

class UVListener : public StorageListener{
private:
    // libuv handler
    void *uv;
    struct sockaddr servaddr;
    Log *log;
    int verbose;
public:
    int status;
    explicit UVListener(
            IdentitySerialization *aIdentitySerialization,
            GatewaySerialization *aSerializationWrapper
    );
    ~UVListener() override;
    void setAddress(
        const std::string &host,
        uint16_t port
    ) override;
    void setAddress(
        uint32_t &ipv4,
        uint16_t port
    ) override;
    int run() override;
    void stop() override;
    void setLog(int verbose, Log *log) override;
};

#endif
