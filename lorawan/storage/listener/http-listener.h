/*
 * @file http-listener.h
 */

#ifndef HTTP_LISTENER_H_
#define HTTP_LISTENER_H_	1

#include <string>
#if defined(_MSC_VER) || defined(__MINGW32__)
#include <WinSock2.h>
#else
#include <sys/socket.h>
#endif

#include "storage-listener.h"

class HTTPListener : public StorageListener {
private:
    uint16_t port;
    Log* log;
public:
    int verbose;
    unsigned int flags;
    unsigned int threadCount;
    unsigned int connectionLimit;
    void *descriptor;   // HTTP daemon
    const char* mimeType;
    std::string htmlRootDir;

    explicit HTTPListener(
        IdentitySerialization* aIdentitySerialization,
        GatewaySerialization* aSerializationWrapper,
        const std::string &aHTMLRootDir = ""
    );
    ~HTTPListener() override;
    void setAddress(
        const std::string& host,
        uint16_t port
    ) override;
    void setAddress(
        uint32_t& ipv4,
        uint16_t port
    ) override;
    int run() override;
    void stop() override;
    void setLog(int verbose, Log* log) override;
};

#endif
