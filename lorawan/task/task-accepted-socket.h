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
    /**
     * @param socket accepted socket
     */
    TaskAcceptedSocket(SOCKET socket);
    /**
    * socket is already open and accepted, so it just return accepted socket
    * @return socket itself
    */
    SOCKET openSocket() override;
    void closeSocket() override;
    virtual ~TaskAcceptedSocket();
};

#endif
