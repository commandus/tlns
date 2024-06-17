#ifndef TASK_UNIX_SOCKET_H_
#define TASK_UNIX_SOCKET_H_ 1

#include <cinttypes>
#include <netinet/in.h>
#include "lorawan/storage/gateway-identity.h"
#include "lorawan/task/task-socket.h"

/**
 * Task Unix domain socket
 */
class TaskUnixSocket : public TaskSocket {
private:
    const char *socketPath;
public:
    TaskUnixSocket(
        const char *socketFileName
    );
    /**
    * Open UDP socket for listen
    * @return -1 if fail
    */
    SOCKET openSocket() override;
    void closeSocket() override;
    // virtual int onData(const char *buffer, size_t size) = 0;
    virtual ~TaskUnixSocket();
};

#endif
