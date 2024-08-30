#ifndef PROTO_GW_JSON_WIRED_CLIENT_H
#define PROTO_GW_JSON_WIRED_CLIENT_H    1

#include "lorawan/lorawan-types.h"
#include "lorawan/proto/gw/proto-gw-parser.h"

class JsonWiredClient {
private:
    const DirectClient *directClient;
    uint64_t gwId;
    std::string networkServerAddress;
    uint16_t networkServerPort;
    DEVADDR deviceAddress;
protected:
    SOCKET sock;
    int openConnection();
    void closeConnection();
public:
    int status;
    JsonWiredClient(
        const DirectClient *directClient,
        uint64_t gwId,
        const std::string& networkServerAddress,
        uint16_t networkServerPort,
        const DEVADDR &deviceAddress
    );
    virtual ~JsonWiredClient();
    int send(
        const std::string &fopts,
        const std::string &payload
    );

    int run();
    void stop();
};

#endif
