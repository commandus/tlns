#ifndef TASK_USB_SOCKET_H_
#define TASK_USB_SOCKET_H_ 1

#include <string>
#include "lorawan/task/task-socket.h"
#include "rak2287.h"
#include "libloragw-helper.h"

class TaskUsbGatewayUnixSocket : public TaskSocket {
private:
    MessageTaskDispatcher *dispatcher;
    std::string socketPath;
    LoraGatewayListener listener;
    bool stopped;
public:
    /**
     * Open Unix domain socket
     * @param socketFileName Unix domain socket name is file name with owner, group access rights e.g. "/tmp/gw-dev-usb.socket"
     * @param devicePath Gateway device file name e.g. "/dev/ttyACM0"
     */
    TaskUsbGatewayUnixSocket(
        MessageTaskDispatcher *dispatcher,
        const std::string &socketFileName,
        GatewaySettings *settings,
        Log *log,
        bool enableSend,
        bool enableBeacon,
        int verbosity
    );
    SOCKET openSocket() override;
    void closeSocket() override;
    ~TaskUsbGatewayUnixSocket() override;
};

#endif
