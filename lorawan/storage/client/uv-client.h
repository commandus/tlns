#ifndef UV_CLIENT_H_
#define UV_CLIENT_H_	1

#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#include <string>
#include <uv.h>
#include "query-client.h"

#define SEND_BUFFER_SIZE 155

class UvClient : public QueryClient {
private:
    char sendBuffer[SEND_BUFFER_SIZE];    // max request size is 40 bytes
    bool useTcp;
    struct sockaddr serverAddress;
    uv_udp_t udpSocket;

    uv_tcp_t tcpSocket;
    uv_connect_t connectReq;

    ServiceMessage* query;

    int status;
    uv_loop_t *loop;

    void init();
    void initiateQuery();

public:
    bool tcpConnected;
    void *dataBuf;
    size_t dataSize;

    explicit UvClient(
            bool useTcp,
            const std::string &aHost,
            uint16_t port,
            ResponseClient *onResponse
    );
    ~UvClient() override;

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
    void finish();
};

#endif
