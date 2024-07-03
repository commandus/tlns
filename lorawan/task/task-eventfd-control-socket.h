#ifndef TASK_EVENTFD_CONTROL_SOCKET_H_
#define TASK_EVENTFD_CONTROL_SOCKET_H_ 1

#include <string>
#include "lorawan/task/task-socket.h"

class TaskEventFDControlSocket : public TaskSocket {
public:
    /**
     * Task eventfd socket
     */
    TaskEventFDControlSocket();
    /**
     * Open Unix domain socket
     * @return socket number, -1 if fails
     */
    SOCKET openSocket() override;
    void closeSocket() override;
    virtual ~TaskEventFDControlSocket();
};

#endif
