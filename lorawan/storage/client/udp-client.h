#ifndef UDP_CLIENT_H_
#define UDP_CLIENT_H_	1

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <BaseTsd.h>
#include <Winsock2.h>
typedef SSIZE_T ssize_t;
#else
#include <netinet/in.h>
typedef int SOCKET;
#endif

#include <string>
#include "lorawan/storage/serialization/gateway-serialization.h"
#include "query-client.h"

class UDPClient : public QueryClient {
private:
    SOCKET sock;
    struct sockaddr addr;
    ServiceMessage* query;
    int status;
public:
    explicit UDPClient(
            uint32_t ipv4,
            uint16_t port,
            ResponseClient *onResponse
    );
    explicit UDPClient(
            const std::string &aHost,
            uint16_t port,
            ResponseClient *onResponse
    );
    ~UDPClient() override;

    /**
     * Prepare to send request
     * @param value
     * @return previous message, NULL if not exists
     */
    ServiceMessage* request(
        ServiceMessage* value
    ) override;
    void start() override;
    void stop() override;
};

#endif
