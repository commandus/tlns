#include <cstring>
#include "ip-address.h"

#include <sstream>
#if defined(_MSC_VER) || defined(__MINGW32__)
#else
#include <arpa/inet.h>
#endif

/**
 * @return address string
 */
std::string sockaddr2string(
    const struct sockaddr *value
) {
    char buf[INET6_ADDRSTRLEN];
    int port;
    switch (value->sa_family) {
        case AF_INET:
            if (inet_ntop(AF_INET, &((struct sockaddr_in *) value)->sin_addr, buf, sizeof(buf)) == nullptr)
                return "";
            port = ntohs(((struct sockaddr_in *) value)->sin_port);
            break;
        case AF_INET6:
            if (inet_ntop(AF_INET6, &((struct sockaddr_in6 *) value)->sin6_addr, buf, sizeof(buf)) == nullptr) {
                return "";
            }
            port = ntohs(((struct sockaddr_in6 *) value)->sin6_port);
            break;
        default:
            return "";
    }
    std::stringstream ss;
    ss << buf << ":" << port;
    return ss.str();
}

/**
 * Split @param address e.g. ADRESS:PORT to @param retAddress and @param retPort
 */
bool splitAddress(
    std::string &retAddress,
    uint16_t &retPort,
    const std::string &address
)
{
    size_t pos = address.find_last_of(':');
    if (pos == std::string::npos)
        return false;
    retAddress = address.substr(0, pos);
    std::string p(address.substr(pos + 1));
    retPort = (uint16_t) strtoul(p.c_str(), nullptr, 0);
    return true;
}

/**
 * Trying parseRX I v6 address, then IPv4
 * @param retval return address into struct sockaddr_in6 struct pointer
 * @param value IPv8 or IPv4 address string
 * @return true if success
 */
bool string2sockaddr(
    struct sockaddr *retval,
    const std::string &value
)
{
    std::string address;
    uint16_t port;
    if (!splitAddress(address, port, value))
        return false;
    return string2sockaddr(retval, address, port);
}

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
)
{
    bool r = inet_pton(AF_INET6, address.c_str(), &((struct sockaddr_in6 *) retval)->sin6_addr) == 1;
    if (r) {
        ((struct sockaddr_in6*) retval)->sin6_family = AF_INET6;
        ((struct sockaddr_in6*) retval)->sin6_port = htons(port);
    } else {
        r = inet_pton(AF_INET, address.c_str(), &((struct sockaddr_in *) retval)->sin_addr) == 1;
        if (r) {
            ((struct sockaddr_in*) retval)->sin_family = AF_INET;
            ((struct sockaddr_in*) retval)->sin_port = htons(port);
        }
    }
    return r;
}

bool sameSocketAddress(
    const struct sockaddr *a,
    const struct sockaddr *b
)
{
    if (a->sa_family != b->sa_family)
        return false;

    switch (a->sa_family) {
        case AF_INET: {
            auto *ai = (struct sockaddr_in *) a;
            auto *bi = (struct sockaddr_in *) b;
            return (ai->sin_port == bi->sin_port) && (memcmp(&ai->sin_addr, &bi->sin_addr, 4) == 0);
        }
        case AF_INET6:
        {
                auto *ai = (struct sockaddr_in6 *) a;
                auto *bi = (struct sockaddr_in6 *) b;
                return (ai->sin6_port == bi->sin6_port) && (memcmp(&ai->sin6_addr, &bi->sin6_addr, 16) == 0);
        }
    }
    return false;
}
