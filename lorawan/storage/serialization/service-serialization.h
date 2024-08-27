#ifndef SERVICE_SERIALIZATION_H
#define SERVICE_SERIALIZATION_H

#include <cstdint>
#include <cstddef>
#include <string>

#define SIZE_SERVICE_MESSAGE   13

class ServiceMessage {
public:
    char tag;
    int32_t code;           // "account#" in request
    uint64_t accessCode;    // magic number in request, retCode in response, negative is error code
    ServiceMessage();
    ServiceMessage(char tag, int32_t code, uint64_t accessCode);
    ServiceMessage(const unsigned char *buf, size_t sz);
    virtual ~ServiceMessage();
    virtual void ntoh();
    virtual size_t serialize(unsigned char *retBuf) const;
    virtual std::string toJsonString() const;
};  // 5 bytes

/**
 * Serialize Internet address v4, v6
 * @param retBuf return buffer
 * @param addr address to serialize
 * @return 0 (unknown family), 7 (IPv4, 19(IPv6)
 */
size_t serializeSocketAddress(
    unsigned char *retBuf,
    const struct sockaddr *addr
);

size_t deserializeSocketAddress(
    struct sockaddr *addr,
    const unsigned char *retBuf,
    size_t sz
);

#endif // SERVICE_SERIALIZATION_H
