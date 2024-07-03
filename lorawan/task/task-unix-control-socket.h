#ifndef TASK_UNIX_CONTROL_SOCKET_H_
#define TASK_UNIX_CONTROL_SOCKET_H_ 1

#include <string>
#include "lorawan/task/task-socket.h"

class TaskUnixControlSocket : public TaskSocket {
private:
    /**
     *  File name e.g. "/tmp/gw-dev-usb.socket"
     *  Please note file has owner, group access rights
     */
    std::string socketPath;
public:
    /**
     * Task Unix domain socket
     * @param socketFileName Unix domain socket name is file name with owner, group access rights e.g. "/tmp/gw-dev-usb.socket"
     */
    TaskUnixControlSocket(
        const std::string &socketFileName
    );
    /**
     * Open Unix domain socket
     * @return socket number, -1 if fails
     */
    SOCKET openSocket() override;
    void closeSocket() override;
    virtual ~TaskUnixControlSocket();
};

#endif
