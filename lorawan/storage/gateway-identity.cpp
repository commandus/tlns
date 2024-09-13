#include "lorawan/storage/gateway-identity.h"
#include <sstream>
#include <iomanip>
#include <cstring>

#include "lorawan/lorawan-date.h"
#include "lorawan/helper/ip-address.h"

/**
 * Create empty gateway statistics
 */
GatewayIdentity::GatewayIdentity()
	: gatewayId(0)
{
    memset(&sockaddr, 0, sizeof(sockaddr));
}

/**
 * Copy gateway statistics
 */
GatewayIdentity::GatewayIdentity(
	const GatewayIdentity &value
)
    : gatewayId(value.gatewayId), sockaddr{}
{
    size_t sz;
    switch (sockaddr.sa_family) {
        case AF_INET:
            sz = sizeof(struct sockaddr_in);
            break;
        case AF_INET6:
            sz = sizeof(struct sockaddr_in6);
            break;
#if defined(_MSC_VER) || defined(__MINGW32__)
        case AF_UNIX:
            sz = sizeof(struct sockaddr_un);
            break;
#endif
        default:
            sz = sizeof(sockaddr);
    }
	memmove(&sockaddr, &value.sockaddr, sz);
}

GatewayIdentity::GatewayIdentity(
    uint64_t aGatewayId
)
    : gatewayId(aGatewayId)
{
    memset(&sockaddr, 0, sizeof(struct sockaddr));
}

GatewayIdentity::GatewayIdentity(
    uint64_t aGatewayId,
    const std::string &addressNport
)
    : gatewayId(aGatewayId)
{
    string2sockaddr(&sockaddr, addressNport);
}

GatewayIdentity::GatewayIdentity(
    uint64_t aGatewayId,
    const std::string &address,
    uint16_t port
)
    : gatewayId(aGatewayId)
{
    string2sockaddr(&sockaddr, address, port);
}

GatewayIdentity::GatewayIdentity(
    const uint64_t aGatewayId,
    const struct sockaddr &addr
)
    : gatewayId(aGatewayId), sockaddr(addr)
{

}

/**
 * 8-bit gateway Identifier
 */
bool GatewayIdentity::operator==(
	GatewayIdentity &rhs
) const {
	return gatewayId == rhs.gatewayId;
}

GatewayIdentity& GatewayIdentity::operator=(
    const GatewayIdentity &value
)
{
    gatewayId = value.gatewayId;
    memmove(&sockaddr, &value.sockaddr, sizeof (struct sockaddr));
    return *this;
}

/**
 * Statistics property names
 */
static const char* STAT_NAMES[13] = {
	"gwid",	// 0 string id
	"addr",	// 1 string address
	"name",	// 2 string name
	"time", // 3 string | UTC time of pkt RX, us precision, ISO 8601 'compact' format
	"lati", // 4 number 
	"long", // 5 number
	"alti", // 6 number
	"rxnb", // 7 number
	"rxok", // 8 number
	"rxfw", // 9 number
	"ackr", // 10 number
	"dwnb", // 11 number
	"txnb" // 12 number
};

/**
 * debug string
 */
std::string GatewayIdentity::toString() const
{
	std::stringstream ss;
	ss << std::hex << gatewayId << " " << sockaddr2string(&sockaddr);
	return ss.str();
}

/**
 * JSON string
 */
std::string GatewayIdentity::toJsonString() const
{
    std::stringstream ss;
    ss << "{"
       << R"("gwid": ")" << std::hex << gatewayId
       << R"(", "addr": ")" << sockaddr2string(&sockaddr)
       << "\"}";
    return ss.str();
}

/**
 * Create empty gateway statistics
 */
GatewayStatistic::GatewayStatistic()
    : GatewayIdentity(), errcode(0),
    t(0),					// UTC time of pkt RX, us precision, ISO 8601 'compact' format
    lat(0.0),				// latitude
    lon(0.0),				// longitude
    alt(0),					// altitude, meters, integer
    rxnb(0),				// Number of radio packets received (unsigned integer)
    rxok(0),				// Number of radio packets received with a valid PHY CRC
    rxfw(0),				// Number of radio packets forwarded (unsigned integer)
    ackr(0.0),				// Percentage of upstream datagrams that were acknowledged
    dwnb(0),				// Number of downlink datagrams received (unsigned integer)
    txnb(0)					// Number of packets emitted (unsigned integer)
{
}

/**
 * Copy gateway statistics
 */
GatewayStatistic::GatewayStatistic(
    const GatewayStatistic &value
)
 : GatewayIdentity(value) {
    name = value.name;
    gatewayId = value.gatewayId;
    errcode = value.errcode;
    t = value.t;
    lat = value.lat;
    lon = value.lon;
    alt = value.alt;
    rxnb = value.rxnb;
    rxok = value.rxok;
    rxfw = value.rxfw;
    ackr = value.ackr;
    dwnb = value.dwnb;
    txnb = value.txnb;
}

/**
 * 8-bit gateway Identifier
 */
bool GatewayStatistic::operator==(
        GatewayStatistic &rhs
) const {
    return gatewayId == rhs.gatewayId;
}


/**
 * debug string
 */
std::string GatewayStatistic::toString() const
{
    std::stringstream ss;
    ss << "{"
        << "\"" << STAT_NAMES[0] << "\": " << std::hex << gatewayId << std::dec
        << ", \"" << STAT_NAMES[1] << "\": " << sockaddr2string(&sockaddr)
        << ", \"" << STAT_NAMES[2] << "\": " << name
        << ", \"" << STAT_NAMES[3] << "\": " << time2string(t)
        << ", \"" << STAT_NAMES[4] << "\": " << std::fixed << std::setprecision(5) << lat
        << ", \"" << STAT_NAMES[5] << "\": " << std::fixed << std::setprecision(5) << lon
        << ", \"" << STAT_NAMES[6] << "\": " << alt
        << ", \"" << STAT_NAMES[7] << "\": " << rxnb
        << ", \"" << STAT_NAMES[8] << "\": " << rxok
        << ", \"" << STAT_NAMES[9] << "\": " << rxfw
        << ", \"" << STAT_NAMES[10] << "\": " << std::fixed << std::setprecision(1) << ackr
        << ", \"" << STAT_NAMES[11] << "\": " << dwnb
        << ", \"" << STAT_NAMES[12] << "\": " << txnb
        << "}";
    return ss.str();
}
