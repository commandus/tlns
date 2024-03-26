#include <string>
#include "lorawan/task/task-socket.h"

class TaskUSBSocket : public TaskSocket {
    std::string path;
public:
    TaskUSBSocket(const std::string &devicePath);
    SOCKET openSocket() override;
    void closeSocket() override;
    // virtual int onData(const char *buffer, size_t size) = 0;
    virtual ~TaskUSBSocket();
};
