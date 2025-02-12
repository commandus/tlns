#ifndef GATEWAY_LISTENER_H
#define GATEWAY_LISTENER_H

#include "lorawan/storage/serialization/identity-serialization.h"
#include "lorawan/storage/serialization/gateway-serialization.h"

class Log {
public:
    virtual std::ostream& strm(int level) = 0;
    virtual void flush() = 0;
};

class StorageListener {
public:
    IdentitySerialization *identitySerialization;
    GatewaySerialization *gatewaySerialization;

    explicit StorageListener(
        IdentitySerialization *aIdentitySerialization,
        GatewaySerialization *aSerializationWrapper
    ) : identitySerialization(aIdentitySerialization), gatewaySerialization(aSerializationWrapper)
    {

    }

    virtual void setAddress(
        const std::string &host,
        uint16_t port
    ) = 0;

    virtual void setAddress(
        uint32_t &ipv4,
        uint16_t port
    ) = 0;

    virtual int run() = 0;

    virtual void stop() = 0;

    virtual void setLog(int verbose, Log *log) = 0;

    virtual ~StorageListener();
};


#endif
