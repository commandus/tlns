#ifndef TASK_SOCKET_H_
#define TASK_SOCKET_H_ 1

#include <cinttypes>
#include <netinet/in.h>
#include "lorawan/storage/gateway-identity.h"

enum ENUM_SOCKET_ACCEPT {
    SA_NONE,
    SA_REQUIRE,
    SA_ACCEPTED
};

class TaskSocket {
public:
    SOCKET sock;
    ENUM_SOCKET_ACCEPT accept;   ///< Protocol family e.g. AF_INET
    int lastError;
    TaskSocket();
    TaskSocket(ENUM_SOCKET_ACCEPT accept);
    TaskSocket(SOCKET socket, ENUM_SOCKET_ACCEPT accept);
    virtual SOCKET openSocket() = 0;
    virtual void closeSocket() = 0;
    // virtual int onData(const char *buffer, size_t size) = 0;
    virtual ~TaskSocket();
};

#endif
