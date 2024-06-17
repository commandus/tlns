#ifndef TASK_UDP_SOCKET_H_
#define TASK_UDP_SOCKET_H_ 1

#include <cinttypes>
#include <netinet/in.h>
#include "lorawan/storage/gateway-identity.h"
#include "lorawan/task/task-socket.h"

/**
 * Task UDP socket
 */
class TaskUDPSocket : public TaskSocket {
private:
    in_addr_t addr;
    uint16_t port;
public:
    /**
     * Create UDP socket. UDP socket does not require accept() call.
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
