#ifndef TASK_ACCEPTED_SOCKET_H_
#define TASK_ACCEPTED_SOCKET_H_ 1

#include <cinttypes>
#include <netinet/in.h>
#include "lorawan/storage/gateway-identity.h"
#include "lorawan/task/task-socket.h"

class TaskAcceptedSocket : public TaskSocket {
public:
    /**
     * @param addr ""- any interface, "localhost"- localhost otherwise- address
     * @param port port number
     */
    TaskAcceptedSocket(SOCKET socket);
    /**
    * Open UDP socket for listen
    * @return -1 if fail
    */
    SOCKET openSocket() override;
    void closeSocket() override;
    virtual ~TaskAcceptedSocket();
};

#endif
