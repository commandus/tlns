#include <string>
#include "lorawan/task/task-socket.h"
#include "rak2287.h"
#include "libloragw-helper.h"

class TaskUSBSocket : public TaskSocket {
private:
    MessageTaskDispatcher *dispatcher;
    std::string socketPath;
    LoraGatewayListener listener;
    LibLoragwOpenClose *helperOpenClose;
    bool stopped;
    int listen2();
public:
    SOCKET controlSocket;
    /**
     * Open Unix domain socket
     * @param socketFileName Unix domain socket name is file name with owner, group access rights e.g. "/tmp/gw-dev-usb.socket"
     * @param devicePath Gateway device file name e.g. "/dev/ttyACM0"
     */
    TaskUSBSocket(
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
    // virtual int onData(const char *buffer, size_t size) = 0;
    virtual ~TaskUSBSocket();
};
