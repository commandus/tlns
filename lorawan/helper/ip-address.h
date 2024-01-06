#ifndef IP_ADDRESS_H
#define IP_ADDRESS_H

#include <string>
#ifdef _MSC_VER
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#define SOCKET int
#include <sys/socket.h>
#include <netinet/in.h>
#endif

/**
 * Split @param address e.g. ADRESS:PORT to @param retAddress and @param retPort
 */
bool splitAddress(
    std::string &retAddress,
    uint16_t &retPort,
    const std::string &address
);

/**
 * Return IP adress:port
 * @return address string
 */
std::string sockaddr2string(
    const struct sockaddr *value
);

/**
 * Trying parseRX I v6 address, then IPv4
 * @param retval return address into struct sockaddr_in6 struct pointer
 * @param value IPv8 or IPv4 address string
 * @return true if success
 */
bool string2sockaddr(
    struct sockaddr *retval,
    const std::string &value
);

/**
 * Trying parseRX I v6 address, then IPv4
 * @param retval return address into struct sockaddr_in6 struct pointer
 * @param address IPv8 or IPv4 address string
 * @param port number
 * @return true if success
 */
bool string2sockaddr(
    struct sockaddr *retval,
    const std::string &address,
    uint16_t port
);

/**
 * Return true if socket address A is same as B
 * @param a socket address A
 * @param b socket address B
 * @return Return true if socket address A is same as B
 */
bool sameSocketAddress(
    const struct sockaddr *a,
    const struct sockaddr *b
);

#endif
