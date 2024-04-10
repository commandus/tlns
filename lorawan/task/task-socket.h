#ifndef TASK_SOCKET_H_
#define TASK_SOCKET_H_ 1

#include <cinttypes>
#include <netinet/in.h>
#include "lorawan/storage/gateway-identity.h"

class TaskSocket {
public:
    SOCKET sock;
    int lastError;
    TaskSocket();
    virtual SOCKET openSocket() = 0;
    virtual void closeSocket() = 0;
    // virtual int onData(const char *buffer, size_t size) = 0;
    virtual ~TaskSocket();
};

#endif
