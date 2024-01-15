#ifndef GATEWAY_IDENTITY_H_
#define GATEWAY_IDENTITY_H_	1

#include <string>
#include <ctime>
#include <cinttypes>
#ifdef _MSC_VER
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
typedef int SOCKET;
#include <sys/socket.h>
#include <netinet/in.h>
#endif

/** 
 * Gateway identity keep current gateway address got from the last PULL request.
 * There are also addr member to keep gateway address read from configuration file. This addr must never used.
 * Gateway identified by 8 bit identifier.
 * 
 * PUSH_DATA (0) stat Section 4
 * {"stat":{"time":"2021-02-24 04:54:01 GMT","lati":62.02774,"long":129.72883,"alti":348,"rxnb":0,"rxok":0,"rxfw":0,"ackr":0.0,"dwnb":0,"txnb":0}}
 */
class GatewayIdentity {
public:
    uint64_t gatewayId;         // gateway identifier
	struct sockaddr sockaddr;   // gateway UIPv4 or IPv6 address, 15-16 bytes long

	GatewayIdentity();
	GatewayIdentity(const GatewayIdentity &value);
    GatewayIdentity(uint64_t gatewayId);
    GatewayIdentity(const uint64_t id, const struct sockaddr &addr);
    GatewayIdentity(uint64_t gatewayId, const std::string &addressNport);
    GatewayIdentity(uint64_t gatewayId, const std::string &address, uint16_t port);
	bool operator==(GatewayIdentity &rhs) const;
    GatewayIdentity& operator=(const GatewayIdentity &value);
	std::string toString() const;
    std::string toJsonString() const;
};

class GatewayStatistic : public GatewayIdentity{
public:
    // gateway common name
    std::string name;
    int errcode;
    time_t t;					// UTC time of pkt RX, us precision, ISO 8601 'expanded' format e.g. 2021-02-24 04:54:01 GMT
    double lat;					// GPS latitude of the gateway in degree (float, N is +)
    double lon;					// GPS longitude of the gateway in degree (float, E is +)
    uint32_t alt;				// altitude, meters, integer
    size_t rxnb;				// Number of radio packets received (unsigned integer)
    size_t rxok;				// Number of radio packets received with a valid PHY CRC
    size_t rxfw;				// Number of radio packets forwarded (unsigned integer)
    double ackr;				// Percentage of upstream datagrams that were acknowledged
    size_t dwnb;				// Number of downlink datagrams received (unsigned integer)
    size_t txnb;				// Number of packets emitted (unsigned integer)

    GatewayStatistic();
    GatewayStatistic(const GatewayStatistic &value);
    bool operator==(GatewayStatistic &rhs) const;
    std::string toString() const;
    std::string toJsonString() const;
};

#endif
