#ifndef TASK_ACCEPTED_SOCKET_H_
#define TASK_ACCEPTED_SOCKET_H_ 1

#include <cinttypes>

#if defined(_MSC_VER) || defined(__MINGW32__)
#else
#include <netinet/in.h>
#endif

#include "lorawan/storage/gateway-identity.h"
#include "lorawan/task/task-socket.h"

/**
 * After TCP socket is accepted, create TaskAcceptedSocket
 */
class TaskAcceptedSocket : public TaskSocket {
public:
    TaskSocket *originator;
    /**
     * @param socket accepted socket
     */
    TaskAcceptedSocket(TaskSocketPreNAcceptedSocket &taskSocketPreNAcceptedSocket);
    /**
    * socket is already open and accepted, so it just return accepted socket
    * @return socket itself
    */
    SOCKET openSocket() override;
    void closeSocket() override;
    virtual ~TaskAcceptedSocket();
    void customWriteSocket(
        const NetworkIdentity *networkIdentity,
        const void* data,
        size_t size,
        ProtoGwParser *proto
    ) override;

};

#endif
