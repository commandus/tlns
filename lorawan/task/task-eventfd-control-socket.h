#ifndef TASK_EVENTFD_CONTROL_SOCKET_H_
#define TASK_EVENTFD_CONTROL_SOCKET_H_ 1

#include <string>
#include "lorawan/task/task-socket.h"

/**
 * Do not use now.
 * Reserved for future use.
 * Can write up to 8 bytes only.
 */
class TaskEventFDControlSocket : public TaskSocket {
public:
    /**
     * Task eventfd socket
     */
    TaskEventFDControlSocket();
    /**
     * Open eventfd socket
     * @return socket number, -1 if fails
     */
    SOCKET openSocket() override;
    void closeSocket() override;
    virtual ~TaskEventFDControlSocket();
};

#endif
