#include <string>
#include "lorawan/task/task-socket.h"

class TaskUnixControlSocket : public TaskSocket {
private:
    std::string socketPath;
public:
    /**
     * Open Unix domain socket
     * @param socketFileName Unix domain socket name is file name with owner, group access rights e.g. "/tmp/gw-dev-usb.socket"
     * @param devicePath Gateway device file name e.g. "/dev/ttyACM0"
     */
    TaskUnixControlSocket(
        const std::string &socketFileName
    );
    SOCKET openSocket() override;
    void closeSocket() override;
    virtual ~TaskUnixControlSocket();
};
