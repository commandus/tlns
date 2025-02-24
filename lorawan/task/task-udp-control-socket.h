#ifndef TASK_UDP_CONTROL_SOCKET_H_
#define TASK_UDP_CONTROL_SOCKET_H_ 1

#include <cinttypes>

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <Winsock2.h>
typedef in_addr in_addr_t;
#else
#include <netinet/in.h>
#endif

#include "lorawan/storage/gateway-identity.h"
#include "lorawan/task/task-socket.h"

class TaskUDPControlSocket : public TaskSocket {
private:
    in_addr_t addr;
    uint16_t port;
public:
    /**
     * @param addr ""- any interface, "localhost"- localhost otherwise- address
     * @param port port number
     */
    TaskUDPControlSocket(in_addr_t aAddr, uint16_t port);
    TaskUDPControlSocket(const std::string &addrNPort);
    /**
    * Open UDP socket for listen
    * @return -1 if fail
    */
    SOCKET openSocket() override;
    void closeSocket() override;
    // virtual int onData(const char *buffer, size_t size) = 0;
    virtual ~TaskUDPControlSocket();
};

#endif
