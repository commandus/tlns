#ifndef TASK_USB_SOCKET_H_
#define TASK_USB_SOCKET_H_ 1

#include <string>
#include "lorawan/task/task-socket.h"
#include "rak2287.h"

class TaskUsbGatewaySocket : public TaskSocket {
private:
    MessageTaskDispatcher *dispatcher;
    std::string socketNameOrAddress;
    LoraGatewayListener listener;
    ProtoGwParser *parser;
public:
#ifdef _MSC_VER
    uint16_t nPort; ///< random assigned UDP port (in network byte order).
#endif
    /**
     * Open Unix domain socket
     * @param socketFileNameOrAddress Unix domain socket name is file name with owner, group access rights e.g. "/tmp/gw-dev-usb.socket"
     * @param devicePath Gateway device file name e.g. "/dev/ttyACM0"
     */
    TaskUsbGatewaySocket(
        MessageTaskDispatcher *dispatcher,
        const std::string &socketFileNameOrAddress,
        GatewaySettings *settings,
        Log *log,
        bool enableSend,
        bool enableBeacon,
        int verbosity
    );
    SOCKET openSocket() override;
    void closeSocket() override;
    ~TaskUsbGatewaySocket() override;
};

#endif
