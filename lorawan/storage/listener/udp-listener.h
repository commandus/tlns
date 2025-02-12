#ifndef UDP_LISTENER_H_
#define UDP_LISTENER_H_	1

#include <string>
#if defined(_MSC_VER) || defined(__MINGW32__)
#include <WinSock2.h>
#else
#include <sys/socket.h>
#endif

#include "lorawan/storage/listener/storage-listener.h"

class UDPListener : public StorageListener {
private:
    struct sockaddr destAddr;
    Log *log;
    int verbose;
public:
    int status; // ERR_CODE_STOPPED - stop request
    explicit UDPListener(
        IdentitySerialization *aIdentitySerialization,
        GatewaySerialization *aSerializationWrapper
    );
    virtual ~UDPListener();
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
