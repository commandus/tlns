#ifndef TASK_SOCKET_H_
#define TASK_SOCKET_H_ 1

#include <cinttypes>
#include <netinet/in.h>
#include "lorawan/storage/gateway-identity.h"

class TaskSocket {
public:
    SOCKET sock;
    int lastError;
    TaskSocket();
    virtual SOCKET openSocket() = 0;
    virtual void closeSocket() = 0;
    // virtual int onData(const char *buffer, size_t size) = 0;
    virtual ~TaskSocket();
};

class TaskUDPSocket : public TaskSocket {
private:
    in_addr_t addr;
    uint16_t port;
public:
    /**
     * @param addr ""- any interface, "localhost"- localhost otherwise- address
     * @param port port number
     */
    TaskUDPSocket(in_addr_t aAddr, uint16_t port);
    /**
    * Open UDP socket for listen
    * @return -1 if fail
    */
    SOCKET openSocket() override;
    void closeSocket() override;
    // virtual int onData(const char *buffer, size_t size) = 0;
    virtual ~TaskUDPSocket();
};

#endif
