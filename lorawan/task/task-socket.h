
#include <cinttypes>
#include <netinet/in.h>
#include "lorawan/storage/gateway-identity.h"

class TaskSocket {
private:
    in_addr_t addr;
    uint16_t port;
public:
    SOCKET sock;
    int lastError;
    /**
     * @param addr ""- any interface, "localhost"- localhost otherwise- address
     * @param port port number
     */
    TaskSocket(in_addr_t aAddr, uint16_t port);
    /**
    * Open UDP socket for listen
    * @return -1 if fail
    */
    SOCKET openSocket();
    void closeSocket();
    // virtual int onData(const char *buffer, size_t size) = 0;
    virtual ~TaskSocket();
};
