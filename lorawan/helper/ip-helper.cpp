#include "ip-helper.h"

#include "lorawan/lorawan-conv.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

void sockaddrNtoh(
    struct sockaddr *addr
)
{
    switch (addr->sa_family)
    {
        case AF_INET6: {
            auto *a = (sockaddr_in6 *) addr;
            NTOH2(a->sin6_port);
        }
        break;
        case AF_INET: {
            auto *a = (sockaddr_in *) addr;
            NTOH2(a->sin_port);
        }
        default:
            break;
    }
}
