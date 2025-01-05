#include <cstring>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <chrono>
#include <algorithm>

#include "lorawan/lorawan-conv.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-date.h"
#include "lorawan/lorawan-mac.h"
#ifdef ENABLE_UNICODE
#include <unicode/unistr.h>
#endif

#define DEF_CODING_RATE CRLORA_4_6

#if defined(_MSC_VER) || defined(__MINGW32__)
#pragma warning(disable: 4996)
#endif

/**
 * @see https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
 */
// trim from start
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends
std::string &trim(std::string &s) {
    ltrim(s);
    rtrim(s);
    return s;
}

// Concatenate two words and place ONE space between them
std::string concatenateWordsWithSpace(
    const std::string &sLeft,
    const std::string &sRight
) {
    std::string s1(sLeft);
    rtrim(s1);
    std::string s2(sRight);
    ltrim(s2);
    return s1 + ' ' + s2;
}

/**
 * @see https://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string
 */
std::string replaceAll(
    std::string str,
    const std::string& from,
    const std::string& to
) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

std::string uint64_t2string(
    const uint64_t &value
) {
    uint64_t v;
    memmove(&v, &value, sizeof(v));
    // hex string is MSB first, swap if need it
    v = NTOH8(v);
    return hexString(&v, sizeof(v));
}

bool isHex(
    const std::string &value
) {
    return value.find_first_not_of("0123456789abcdefABCDEF") == std::string::npos;
}

bool isDec(
    const std::string &value
) {
    return !value.empty()
        && std::find_if(value.begin(), value.end(),
        [] (unsigned char c) {
            return !std::isdigit(c);
        }
    ) == value.end();
}

// http://stackoverflow.com/questions/673240/how-do-i-print-an-unsigned-char-as-hex-in-c-using-ostream
struct HexCharStruct
{
    unsigned char c;
    explicit HexCharStruct(unsigned char _c) : c(_c) { }
};

inline std::ostream& operator<<(std::ostream& o, const HexCharStruct& hs)
{
    return (o << std::setfill('0') << std::setw(2) << std::hex << (int) hs.c);
}

inline HexCharStruct hex(
        unsigned char c
)
{
    return HexCharStruct(c);
}

static void bufferPrintHex(
        std::ostream &ostream,
        const void* value, size_t size
)
{
    if (!value)
        return;
    auto *p = (unsigned char*) value;
    for (size_t i = 0; i < size; i++)
    {
        ostream << hex(*p);
        p++;
    }
}

std::string hexString(
    const void *buffer,
    size_t size
)
{
    std::stringstream r;
    bufferPrintHex(r, buffer, size);
    return r.str();
}

/**
 * Return hex string
 * @param data
 * @return
 */
std::string hexString(
    const std::string &data
)
{
    return hexString((void *) data.c_str(), data.size());
}

static std::string readHex(
    std::istream &s
)
{
    std::stringstream r;
    s >> std::noskipws;
    char c[3] = {0, 0, 0};
    while (s >> c[0]) {
        if (!(s >> c[1]))
            break;
        auto x = (unsigned char) strtol(c, nullptr, 16);
        r << x;
    }
    return r.str();
}

std::string hex2string(
    const std::string &hex
)
{
    std::stringstream ss(hex);
    return readHex(ss);
}

std::string toUpperCase(
    const std::string &value
)
{
    std::string r;
#ifdef ENABLE_UNICODE
    icu::UnicodeString::fromUTF8(value).toUpper().toUTF8String(r);
#else
    r = value;
    for (auto & c: r) {
        c = std::toupper(c);
    }
#endif
#if defined(_MSC_VER) || defined(__MINGW32__)
    if (r.empty())
        CharUpperA((LPSTR)r.c_str());
#endif
    if (r.empty())
        return value;
    else
        return r;
}

std::string firstCharToUpperCase(
    const std::string &value
)
{
    std::string r(value);
    if (!r.empty())
        r[0] = std::toupper(r[0]);
    return r;
}

std::string DEVICENAME2string(
    const DEVICENAME &value
)
{
    size_t sz = strnlen(value.c, sizeof(DEVICENAME::c));
    return std::string(value.c, sz);
}

std::string gatewayId2str(
    uint64_t value
) {
    std::stringstream ss;
    ss << std::hex << value;
    return ss.str();
}

std::string MHDR2String(
        const MHDR &value
)
{
    std::stringstream ss;
    ss << "{\"mtype\": " << (int) value.f.mtype
       << ", \"major\": " << (int) value.f.major
       << ", \"rfu\": " << (int) value.f.rfu
       << "}";
    return ss.str();
}

std::string MIC2String(
    uint32_t value
)
{
    // hex string is MSB first, swap if need it
    value = NTOH4(value);
    return hexString(&value, sizeof(value));
}

std::string DEVADDR2string(
    const DEVADDR &value
)
{
    uint32_t v = value.u;
    // hex string is MSB first
    v = SWAP_BYTES_4(v);
    return hexString(&v, sizeof(v));
}

std::string DEVEUI2string(
    const DEVEUI &value
)
{
    // EUI stored in memory as 8-bit integer x86 LSB first, ARM MSB first
    uint64_t v;
    memmove(&v, &value, sizeof(DEVEUI));
    // hex string is MSB first, swap if need it
    v = SWAP_BYTES_8(v);
    return hexString(&v, sizeof(v));
}

std::string KEY2string(
    const KEY128 &value
)
{
    return hexString(&value, sizeof(value));
}

std::string DEVNONCE2string(
    const DEVNONCE &value
)
{
    return hexString(&value, sizeof(value));
}

std::string JOINNONCE2string(
    const JOINNONCE &value
)
{
    return hexString(&value, sizeof(value));
}

DEVNONCE string2DEVNONCE(
    const std::string &value
)
{
    DEVNONCE r;
    r.u = (uint16_t) strtoul(value.c_str(), nullptr, 16);
    r.u = NTOH2(r.u);
    return r;
}

std::string JOIN_ACCEPT_FRAME_HEADER2string(
        const JOIN_ACCEPT_FRAME_HEADER &value
) {
    std::stringstream ss;
    ss << R"({"joinNonce": ")" << JOINNONCE2string(value.joinNonce)
       << R"(", "netId": ")" << NETID2String(value.netId)
       << R"(", "devAddr": ")" << DEVADDR2string(value.devAddr)
       << R"(", "dlSettings": {)"
       << "\"RX2DataRate\": " << (int) value.dlSettings.RX2DataRate	    ///< downlink data rate that serves to communicate with the end-device on the second receive window (RX2)
       << ", \"RX1DROffset\": " << (int) value.dlSettings.RX1DROffset   ///< offset between the uplink data rate and the downlink data rate used to communicate with the end-device on the first receive window (RX1)
       << ", \"optNeg\": " << (int) value.dlSettings.optNeg     	    ///< 1.0- RFU, 1.1- optNeg
       << R"(}, "rxDelay": ")" << (int) value.rxDelay
       << "\"}";
    return ss.str();
}

std::string JOIN_ACCEPT_FRAME2string(
        const JOIN_ACCEPT_FRAME &value
) {
    std::stringstream ss;
    ss << "{\"header\": " << JOIN_ACCEPT_FRAME_HEADER2string(value.hdr)
       << R"(, "mic": ")" << MIC2String(value.mic) << "\"}";
    return ss.str();
}

std::string CFLIST2string(
        const CFLIST &value
)
{
    std::stringstream ss;

    ss << "{\"cflisttype\": " << (int) value.cflisttype
       << ", \"frequency\": [";
    for (int i = 0; i < 5; i++) {
        ss << FREQUENCY2int(value.frequency[i]);
        if (i < 4)
            ss << ", ";
    }
    ss << "]}";
    return ss.str();
}

std::string JOIN_REQUEST_FRAME2string(
        const JOIN_REQUEST_FRAME &value
) {
    return R"({"joinEUI": ")" + DEVEUI2string(value.joinEUI) + "\", "
           + R"("devEUI": ")" + DEVEUI2string(value.devEUI) + "\", "
           + R"("devNonce": ")" + DEVNONCE2string(value.devNonce) + "\"}";
}

std::string JOIN_ACCEPT_FRAME_CFLIST2string(
        const JOIN_ACCEPT_FRAME_CFLIST &value
) {
    std::stringstream ss;
    ss << "{\"header\": " << JOIN_ACCEPT_FRAME_HEADER2string(value.hdr) << ", "
       << ", \"cflist\": " << CFLIST2string(value.cflist) << ", "
       << R"(, "mic": ")" << MIC2String(value.mic) << "\"}";
    return ss.str();
}

std::string DOWNLINK_STORAGE2String(
    const DOWNLINK_STORAGE &value,
    int size
)
{
    std::stringstream ss;
    int payloadSize = size - value.f.foptslen;
    if (payloadSize > 255)
        payloadSize = 255;
    ss << R"({"addr": ")" << DEVADDR2string(value.devaddr)
        << R"(", "foptslen": )" << (int) value.f.foptslen
        << ", \"fpending\": " << (value.f.fpending ? "true" : "false")
        << ", \"ack\": " << (value.f.ack ? "true" : "false")
        << ", \"rfu\": " << (int) value.f.rfu
        << ", \"adr\": " << (value.f.adr ? "true" : "false")
        << ", \"fcnt\": " << value.fcnt;

    if (value.f.foptslen) {
        ss << R"(, "fopts": ")" << hexString((const char *) value.fopts(), (int) value.f.foptslen);
        MacPtr macPtr((const char *) value.fopts(), value.f.foptslen, true);
        ss << R"(", "mac": )" << (macPtr.toJSONString());
        if (macPtr.errorcode) {
            // ignore
        }
    }
    if (payloadSize) {
        ss
            << R"(, "fport": ")" << (int) value.fport()
            << R"(", "payload": ")" << hexString((const char *) value.payload(), payloadSize)
            << "\"";
    }
    ss << "}";
    return ss.str();
}

std::string UPLINK_STORAGE2String(
    const UPLINK_STORAGE &value,
    int size
)
{
    std::stringstream ss;
    int payloadSize = size - value.f.foptslen;
    if (payloadSize > 255)
        payloadSize = 255;
    else
    if (payloadSize < 0)
        payloadSize = 0;
    ss << R"({"addr": ")" << DEVADDR2string(value.devaddr)
        << R"(", "foptslen": )" << (int) value.f.foptslen
        << ", \"classb\": " << (value.f.classb ? "true" : "false")
        << ", \"ack\": " << (value.f.ack ? "true" : "false")
        << ", \"addrackreq\": " << (int) value.f.addrackreq
        << ", \"adr\": " << (value.f.adr ? "true" : "false")
        << ", \"fcnt\": " << value.fcnt;
    if (value.f.foptslen) {
        ss << R"(, "fopts": ")" << hexString((const char *) value.fopts(), (int) value.f.foptslen);
        MacPtr macPtr((const char *) value.fopts(), value.f.foptslen, false);
        ss << R"(", "mac": )" << (macPtr.toJSONString());
        if (macPtr.errorcode) {
            // ignore
        }
    }
    if (payloadSize) {
        ss
            << R"(, "fport": ")" << (int) value.fport()
            << R"(", "payload": ")" << hexString((const char *) value.payload(), payloadSize)
            << "\"";
    }
    ss << "}";
    return ss.str();
}

std::string NETID2String(
        const NETID &value
)
{
    uint32_t r = NETID2int(value);
    std::stringstream ss;
    ss << std::hex << std::setw(6) << std::setfill('0') << r;
    return ss.str();
}

static const char *ACTIVATION_NAMES[2] = {
        "ABP",
        "OTAA"
};

std::string activation2string(
    ACTIVATION value
)
{
    if ((unsigned int) value > OTAA)
        value = ABP;
    return ACTIVATION_NAMES[value];
}

std::string MODULATION2String(
    MODULATION value
)
{
    switch (value) {
        case MODULATION_LORA:
            return "LORA";
        case MODULATION_FSK:
            return "FSK";
        default:
            return "UNDEFINED";
    }
}

std::string BANDWIDTH2String(
    BANDWIDTH value
) {
    switch (value) {
        case BANDWIDTH_INDEX_7KHZ:
            return "7.8";
        case BANDWIDTH_INDEX_10KHZ:
            return "10.4";
        case BANDWIDTH_INDEX_15KHZ:
            return "15.6";
        case BANDWIDTH_INDEX_20KHZ:
            return "20.8";
        case BANDWIDTH_INDEX_31KHZ:
            return "31.2";
        case BANDWIDTH_INDEX_41KHZ:
            return "41.6";
        case BANDWIDTH_INDEX_62KHZ:
            return "62.5";
        case BANDWIDTH_INDEX_125KHZ:
            return "125";
        case BANDWIDTH_INDEX_250KHZ:
            return "250";
        case BANDWIDTH_INDEX_500KHZ:
            return "500";
    }
    return "7.8";
}

std::string LORAWAN_VERSION2string(
        LORAWAN_VERSION value
)
{
    std::stringstream ss;
    ss  << (int) value.major
        << "." << (int) value.minor
        << "." << (int) value.release;
    return ss.str();
}

std::string deviceclass2string(
        DEVICECLASS value
) {
    switch(value) {
        case CLASS_A:
            return "A";
        case CLASS_B:
            return "B";
        default:
            return "C";
    }
}


MTYPE string2mtype(
        const std::string &value
) {
    if (value == "join-request")
        return MTYPE_JOIN_REQUEST;
    if (value == "join-accept")
        return MTYPE_JOIN_ACCEPT;
    if (value == "unconfirmed-data-up")
        return MTYPE_UNCONFIRMED_DATA_UP;
    if (value == "unconfirmed-data-down")
        return MTYPE_UNCONFIRMED_DATA_DOWN;
    if (value == "confirmed-data-up")
        return MTYPE_CONFIRMED_DATA_UP;
    if (value == "confirmed-data-down")
        return MTYPE_CONFIRMED_DATA_DOWN;
    if (value == "rejoin-request")
        return MTYPE_REJOIN_REQUEST;
    if (value == "proprietary-radio")
        return MTYPE_PROPRIETARYRADIO;
    return MTYPE_JOIN_REQUEST;	//?!!
}

std::string mtype2string(
        MTYPE value
)
{
    switch (value) {
        case MTYPE_JOIN_REQUEST:
            return "join-request";
        case MTYPE_JOIN_ACCEPT:
            return "join-accept";
        case MTYPE_UNCONFIRMED_DATA_UP:
            return "unconfirmed-data-up";
        case MTYPE_UNCONFIRMED_DATA_DOWN:
            return "unconfirmed-data-down";
        case MTYPE_CONFIRMED_DATA_UP:
            return "confirmed-data-up";
        case MTYPE_CONFIRMED_DATA_DOWN:
            return "confirmed-data-down";
        case MTYPE_REJOIN_REQUEST:
            return "rejoin-request";
        case MTYPE_PROPRIETARYRADIO:
            return "proprietary-radio";
        default:
            return "";
    }
}

MHDR string2mhdr(
        const std::string &value
)
{
    MHDR r{};
    r.f.mtype = string2mtype(value);
    return r;
}

std::string mhdr2string(
        MHDR value
)
{
    return mtype2string((MTYPE) value.f.mtype);
}

static bool isDownlink(
        MHDR mhdr
)
{
    return ((mhdr.f.mtype == MTYPE_UNCONFIRMED_DATA_DOWN) || (mhdr.f.mtype == MTYPE_CONFIRMED_DATA_DOWN));
}

static bool isUplink(
        MHDR mhdr
)
{
    return ((mhdr.f.mtype == MTYPE_UNCONFIRMED_DATA_UP) || (mhdr.f.mtype == MTYPE_CONFIRMED_DATA_UP));
}

#define DLMT    ", "

std::string fctrl2string(
    const RFM_HEADER* hdr
)
{
    if (!hdr)
        return "";
    std::stringstream ss;
    // frame-options length actual length of FOpts
    ss << (unsigned int) hdr->fhdr.fctrl.f.foptslen << DLMT;
    // 1- gateway has more data pending to be sent
    if (isDownlink(hdr->macheader))
        ss << (hdr->fhdr.fctrl.f.fpending == 0?"not-":"") << "pending" << DLMT;

    if (isUplink(hdr->macheader))
        ss << (hdr->fhdr.fctrl.fup.classb == 0?"no-":"") << "classB" << DLMT;
    ss << (hdr->fhdr.fctrl.f.ack == 0?"no ":"") << "ACK" << DLMT;
    // validate that the network still receives the uplink frames.
    if (isUplink(hdr->macheader))
        ss << (hdr->fhdr.fctrl.fup.addrackreq == 0?"no-":"") << "addrACKrequest" << DLMT;
    // network will control the data rate of the end-device through the MAC commands
    ss << (hdr->fhdr.fctrl.f.adr == 0?"no ":"") << "adr";
    return ss.str();
}

/**
 * MAC command identifier (CID)
 * @param cid MAC command identifier
 * @param foptSize
 * @param bufferSize
 * @return
 */
static std::string cid2string(
        char cid
)
{
    return getMACCommandName(cid);
}

std::string mac2string(
        void *value,
        uint8_t foptSize,
        size_t bufferSize
)
{
    size_t sz = foptSize;
    if (sz > bufferSize)
        sz = bufferSize;
    if (!sz)
        return "";
    return hexString((char *) value, sz) + " (" + cid2string(*(const char *) value) + ")";
}

std::string rfm_header2string(
        const RFM_HEADER* value
)
{
    std::stringstream ss;
    ss  << mhdr2string(value->macheader) << DLMT
        << DEVADDR2string(value->fhdr.devaddr) << DLMT
        << fctrl2string(value) << DLMT
        << value->fhdr.fcnt;     // frame counter
    return ss.str();
}

ACTIVATION string2activation(
    const std::string &value
)
{
    if (value == "OTAA")
        return OTAA;
    else
        return ABP;
}

ACTIVATION pchar2activation(
    const char *name
)
{
    for (int i = 0; i < 2; i++) {
        if (strcmp(ACTIVATION_NAMES[i], name) == 0)
            return (ACTIVATION) i;
    }
    // default ABP
    return ABP;
}

MODULATION string2MODULATION(
    const char *value
)
{
    if (strcmp(value, "FSK") == 0)
        return MODULATION_FSK;
    else
        if (strcmp(value, "LORA") == 0)
            return MODULATION_LORA;
    return MODULATION_UNDEFINED;
}

BANDWIDTH string2BANDWIDTH(
    const char *value
)
{
    if (strcmp(value, "7.8") == 0)
        return BANDWIDTH_INDEX_7KHZ;
    if (strcmp(value, "10.4") == 0)
        return BANDWIDTH_INDEX_10KHZ;
    if (strcmp(value, "15.6") == 0)
        return BANDWIDTH_INDEX_15KHZ;
    if (strcmp(value, "20.8") == 0)
        return BANDWIDTH_INDEX_20KHZ;
    if (strcmp(value, "31.2") == 0)
        return BANDWIDTH_INDEX_31KHZ;
    if (strcmp(value, "41.6") == 0)
        return BANDWIDTH_INDEX_41KHZ;
    if (strcmp(value, "62.5") == 0)
        return BANDWIDTH_INDEX_62KHZ;
    if (strcmp(value, "125") == 0)
        return BANDWIDTH_INDEX_125KHZ;
    if (strcmp(value, "250") == 0)
        return BANDWIDTH_INDEX_250KHZ;
    if (strcmp(value, "500") == 0)
        return BANDWIDTH_INDEX_500KHZ;
    return BANDWIDTH_INDEX_7KHZ;
}

LORAWAN_VERSION string2LORAWAN_VERSION(
        const std::string &value
)
{
    std::stringstream ss(value);
    int ma = 1, mi = 0, re = 0;
    char dot;
    if (!ss.eof ())
        ss >> ma;
    if (!ss.eof ())
        ss >> dot;
    if (!ss.eof ())
        ss >> mi;
    if (!ss.eof ())
        ss >> dot;
    if (!ss.eof ())
        ss >> re;
    LORAWAN_VERSION r = { (uint8_t) (ma & 3), (uint8_t) (mi & 3), (uint8_t) (re & 0xf) };
    return r;
}


DEVICECLASS string2deviceclass(
    const std::string &value
)
{
    if (value.empty())
        return CLASS_C;
    if (value[0] == 'A' || value[0] == 'a')
        return CLASS_A;
    else
        if (value[0] == 'B' || value[0] == 'b')
            return CLASS_B;
        else
            return CLASS_C;
}

void string2DEVADDR(
        DEVADDR &retVal,
        const char *value
)
{
    retVal.u = strtoul(value, nullptr, 16);
}

void string2DEVADDR(
        DEVADDR &retVal,
        const std::string &value
)
{
    retVal.u = strtoul(value.c_str(), nullptr, 16);
}

void string2DEVEUI(
    DEVEUI &retval,
    const std::string &value
)
{
    retval.u = strtoull(value.c_str(), nullptr, 16);
}

void string2DEVEUI(
    DEVEUI &retval,
    const char *value
)
{
    retval.u = strtoull(value, nullptr, 16);
}

void string2KEY(
        KEY128 &retVal,
        const char *str
)
{
    char c[3] = {0, 0, 0};
    int i = 0;
    while (*str) {
        c[0] = *str;
        str++;
        if (!*str)
            break;
        c[1] = *str;
        retVal.c[i] = (unsigned char) strtol(c, nullptr, 16);
        i++;
        if (i > 15)
            break;
        str++;
    }
}

void string2KEY(
        KEY128 &retVal,
        const std::string &str
)
{
    string2KEY(retVal, str.c_str());
}

void string2DEVICENAME(
        DEVICENAME &retval,
        const char *str
)
{
    strncpy(retval.c, str, sizeof(DEVICENAME::c));
}

void string2JOINNONCE(
        JOINNONCE &retval,
        const std::string &value
)
{
    uint32_t r = strtol(value.c_str(), nullptr, 16);
    retval.c[2] = r & 0xff;
    retval.c[1] = (r >> 8) & 0xff;
    retval.c[0] = (r >> 16) & 0xff;
}

void string2APPNONCE(
        APPNONCE& retval,
        const std::string& value
)
{
    uint32_t r = strtol(value.c_str(), nullptr, 16);
    retval.c[2] = r & 0xff;
    retval.c[1] = (r >> 8) & 0xff;
    retval.c[0] = (r >> 16) & 0xff;
}

void string2NETID(
        NETID &retVal,
        const char *value
) {
    std::string str = hex2string(value);
    size_t len = str.size();
    if (len > sizeof(NETID))
        len = sizeof(NETID);
    memmove(&retVal.c, str.c_str(), len);
    if (len < sizeof(NETID))
        memset(&retVal.c + len, 0, sizeof(NETID) - len);
}

void string2FREQUENCY(
        FREQUENCY &retVal,
        const char *value
) {
    std::string str = hex2string(value);
    size_t len = str.size();
    if (len > sizeof(FREQUENCY))
        len = sizeof(FREQUENCY);
    memmove(&retVal, str.c_str(), len);
    if (len < sizeof(FREQUENCY))
        memset(&retVal + len, 0, sizeof(FREQUENCY) - len);
}

std::string frequency2string(
    const FREQUENCY &value
)
{
    return freq2string(FREQUENCY2int(value));
}

std::string freq2string(
    const uint32_t freq
)
{
    std::stringstream ss;
    int mhz = (int) freq / 1000000;
    ss << mhz << "." << (freq - (mhz * 1000000));
    return ss.str();
}

uint64_t string2gatewayId(
        const std::string& value
)
{
    return strtoull(value.c_str(), nullptr, 16);
}

static void setIdentity(
    NETWORKIDENTITY &retVal,
    NETWORK_IDENTITY_PROPERTY property,
    char *start,
    char *finish
) {
    std::string s(start, finish - start);
    switch (property) {
        case NIP_ADDRESS:
            string2DEVADDR(retVal.value.devaddr, s);
            break;
        case NIP_ACTIVATION:
            retVal.value.devid.id.activation = string2activation(s);
            break;
        case NIP_DEVICE_CLASS:
            retVal.value.devid.id.deviceclass = string2deviceclass(s);
            break;
        case NIP_DEVEUI:
            string2DEVEUI(retVal.value.devid.id.devEUI, s);
            break;
        case NIP_NWKSKEY:
            string2KEY(retVal.value.devid.id.nwkSKey, s);
            break;
        case NIP_APPSKEY:
            string2KEY(retVal.value.devid.id.appSKey, s);
            break;
        case NIP_LORAWAN_VERSION:
            retVal.value.devid.id.version = string2LORAWAN_VERSION(s);
            break;
        case NIP_APPEUI:
            string2DEVEUI(retVal.value.devid.id.appEUI, s);
            break;
        case NIP_APPKEY:
            string2KEY(retVal.value.devid.id.appKey, s);
            break;
        case NIP_NWKKEY:
            string2KEY(retVal.value.devid.id.nwkKey, s);
            break;
        case NIP_DEVNONCE:
            retVal.value.devid.id.devNonce = string2DEVNONCE(s);
            break;
        case NIP_JOINNONCE:
            string2JOINNONCE(retVal.value.devid.id.joinNonce, s);
            break;
        case NIP_DEVICENAME:
            string2DEVICENAME(retVal.value.devid.id.name, s.c_str());
            break;
        default:
            break;
    }
}

bool string2NETWORKIDENTITY(
    NETWORKIDENTITY &retVal,
    const char *identityString
)
{
    int c = (int) NIP_ADDRESS;
    char *start = (char *) identityString;
    char *p = start;
    for (; *p != 0; p++) {
        if (*p == ',') {
            setIdentity(retVal, (NETWORK_IDENTITY_PROPERTY) c, start, p);
            c++;
            start = p + 1;
        }
    }
    setIdentity(retVal, (NETWORK_IDENTITY_PROPERTY) c, start, p);
    return true;
}

const std::string ERR_CODE_TX_STR[] {
        "NONE",             // 0
        "TOO_LATE",
        "TOO_EARLY",
        "FULL",
        "EMPTY",
        "COLLISION_PACKET", // 5
        "COLLISION_BEACON",
        "TX_FREQ",
        "TX_POWER",
        "GPS_UNLOCKED",
        "INVALID"           // 10
};

const std::string& ERR_CODE_TX2string(
        ERR_CODE_TX code
)
{
    if (code > JIT_TX_ERROR_INVALID)
        code = JIT_TX_ERROR_INVALID;
    return ERR_CODE_TX_STR[code];
}

ERR_CODE_TX string2ERR_CODE_TX(
        const std::string &value
)
{
    for (int c = 0; c <= JIT_TX_ERROR_INVALID; c++) {
        if (ERR_CODE_TX_STR[c] == value)
            return (ERR_CODE_TX) c;
    }
    return JIT_TX_ERROR_INVALID;
}

/**
 * Parse data rate identifier e.g."SF7BW125" into bandwidth & spreading factor variables
 * @param bandwidth return bandwidth index
 * @param value LoRa datarate identifier e.g. "SF7BW125"
 * @returns preading factor
 */
SPREADING_FACTOR string2datr(
        BANDWIDTH &bandwidth,
        const std::string &value
)
{
    size_t sz = value.size();
    if (sz < 3)
        return DRLORA_SF5;
    std::size_t p = value.find('B');
    if (p == std::string::npos)
        return DRLORA_SF5;
    std::string s = value.substr(2, p - 2);
    auto spreadingFactor = static_cast<SPREADING_FACTOR>(strtol(s.c_str(), nullptr, 10));
    s = value.substr(p + 2);
    int bandwidthValue = strtol(s.c_str(), nullptr, 10);
    switch (bandwidthValue) {
        case 7:
            bandwidth = BANDWIDTH_INDEX_7KHZ; // 7.8
            break;
        case 10:
            bandwidth = BANDWIDTH_INDEX_10KHZ; // 10.4
            break;
        case 15:
            bandwidth = BANDWIDTH_INDEX_15KHZ; // 15.6
            break;
        case 20:
            bandwidth = BANDWIDTH_INDEX_20KHZ; // 20.8
            break;
        case 31:
            bandwidth = BANDWIDTH_INDEX_31KHZ; // 31.2
            break;
        case 41:
            bandwidth = BANDWIDTH_INDEX_41KHZ; // 41.6
            break;
        case 62:
            bandwidth = BANDWIDTH_INDEX_62KHZ; // 62.5
            break;
        case 125:
            bandwidth = BANDWIDTH_INDEX_125KHZ; // 125
            break;
        case 250:
            bandwidth = BANDWIDTH_INDEX_250KHZ;
            break;
        case 500:
            bandwidth = BANDWIDTH_INDEX_500KHZ;
            break;
        default:
            bandwidth = BANDWIDTH_INDEX_250KHZ;
            break;
    }
    return spreadingFactor;
}

/**
 * Return data rate identifier
 * @return LoRa datarate identifier e.g. "SF7BW125"
 */
std::string datr2string(
        SPREADING_FACTOR spreadingFactor,
        BANDWIDTH bandwidth
)
{
    int bandwidthValue;
    switch (bandwidth) {
        case BANDWIDTH_INDEX_7KHZ:
            bandwidthValue = 7; // 7.8
            break;
        case BANDWIDTH_INDEX_10KHZ:
            bandwidthValue = 10; // 10.4
            break;
        case BANDWIDTH_INDEX_15KHZ:
            bandwidthValue = 15; // 15.6
            break;
        case BANDWIDTH_INDEX_20KHZ:
            bandwidthValue = 20; // 20.8
            break;
        case BANDWIDTH_INDEX_31KHZ:
            bandwidthValue = 31; // 31.2
            break;
        case BANDWIDTH_INDEX_41KHZ:
            bandwidthValue = 41; // 41.6
            break;
        case BANDWIDTH_INDEX_62KHZ:
            bandwidthValue = 62; // 62.5
            break;
        case BANDWIDTH_INDEX_125KHZ:
            bandwidthValue = 125; // 125
            break;
        case BANDWIDTH_INDEX_250KHZ:
            bandwidthValue = 250; // 250
            break;
        case BANDWIDTH_INDEX_500KHZ:
            bandwidthValue = 500; // 500
            break;
        default:
            bandwidthValue = 250;
            break;
    }
    std::stringstream ss;
    // e.g. SF7BW203
    ss << "SF" << (int) spreadingFactor
       << "BW" << bandwidthValue;
    return ss.str();
}

/**
 * @param LoRa LoRa ECC coding rate identifier e.g. "4/6"
 */
CODING_RATE string2codingRate(
    const std::string &value
)
{
    size_t sz = value.size();
    switch (sz) {
        case 3:
            switch(value[2]) {
                case '5':
                    return CRLORA_4_5;
                case '3':   // "2/3"
                case '6':
                    return CRLORA_4_6;
                case '7':
                    return CRLORA_4_7;
                case '1':   // "1/2"
                case '4':
                    return CRLORA_4_8;
            }
            break;
        case 5:
            switch(value[2]) {
                case '5':
                    return CRLORA_LI_4_5;
                case '6':
                    return CRLORA_LI_4_6;
                case '8':
                    return CRLORA_LI_4_8;
            }
            break;
    }
    return DEF_CODING_RATE;
}

std::string codingRate2string(
        CODING_RATE codingRate
)
{
    switch (codingRate) {
        case CRLORA_0FF:
            return "";
        case CRLORA_4_5:
            return "4/5";
        case CRLORA_4_6:
            return "4/6";
        case CRLORA_4_7:
            return "4/7";
        case CRLORA_4_8:
            return "4/8";
        case CRLORA_LI_4_5:
            return "4/5LI";
        case CRLORA_LI_4_6:
            return "4/6LI";
        case CRLORA_LI_4_8:
            return "4/8LI";
    }
    return "4/6";
}

std::string SEMTECH_PROTOCOL_METADATA_RX2string(
        const SEMTECH_PROTOCOL_METADATA_RX &value
)
{
    std::stringstream ss;
    ss << R"({"gatewayId": ")" << gatewayId2str(value.gatewayId)
       << R"(", "time": ")" << time2string(value.t)
       << R"(", "tmst": )" << value.tmst
       << ", \"chan\": " << (int) value.chan
       << ", \"rfch\": " << (int) value.rfch
       << ", \"freq\": " << value.freq
       << ", \"stat\": " << (int) value.stat
       << R"(, "modu": ")" << MODULATION2String(value.modu)
       << R"(", "datr": ")" <<  datr2string(value.spreadingFactor, value.bandwidth)
       << R"(", "codr": ")" << codingRate2string(value.codingRate)
       << R"(", "bps": )" << value.bps
       << ", \"rssi\": " << value.rssi
       << ", \"lsnr\": " << std::fixed << std::setprecision(2) << value.lsnr
       << "}";
    return ss.str();
}

std::string SEMTECH_PROTOCOL_METADATA_TX2string(
    const SEMTECH_PROTOCOL_METADATA_TX &value
)
{
    std::stringstream ss;
    ss  << "{\"freq\": " << std::fixed << std::setprecision(6) << value.freq_hz
        << ", \"tx_mode\": " << (int) value.tx_mode
        << ", \"count_us\": " << value.count_us
        << ", \"rfch\": " << (int) value.rf_chain
        << ", \"powe\": " << (int) value.rf_power
        << R"(, "modu": ")" << MODULATION2String((MODULATION) value.modulation)
        << R"(", "bandwidth": )" << BANDWIDTH2String((BANDWIDTH) value.bandwidth)
        << R"(, "codr": ")" << codingRate2string((CODING_RATE) value.coderate)
        << R"(", "ipol": )" << (value.invert_pol? "true" : "false")
        << ", \"fdev\": " << (int) value.f_dev
        << ", \"prea\": " << value.preamble
        << ", \"ncrc\": " << (value.no_crc? "true" : "false")
        << ", \"no_header\": " << (value.no_header? "true" : "false")
        << ", \"size\": " << value.size
        << "}";
    return ss.str();
}

std::string REGIONAL_PARAMETERS_VERSION2string(
        REGIONAL_PARAMETERS_VERSION value
) {
    return LORAWAN_VERSION2string(*(LORAWAN_VERSION*) &value);
}

REGIONAL_PARAMETERS_VERSION string2REGIONAL_PARAMETERS_VERSION(
        const std::string &value
) {
    std::stringstream ss(value);
    int ma = 1, mi = 0, re = 0;
    char dot;
    if (!ss.eof ())
        ss >> ma;
    if (!ss.eof ())
        ss >> dot;
    if (!ss.eof ())
        ss >> mi;
    if (!ss.eof ())
        ss >> dot;
    if (!ss.eof ())
        ss >> re;
    REGIONAL_PARAMETERS_VERSION r = { (uint8_t) (ma & 3), (uint8_t) (mi & 3), (uint8_t) (re & 0xf) };
    return r;
}

static std::string file2string(
    std::istream &strm
)
{
    if (!strm)
        return "";
    return std::string((std::istreambuf_iterator<char>(strm)), std::istreambuf_iterator<char>());
}

std::string file2string(
    const char *filename
)
{
    if (!filename)
        return "";
    std::ifstream t(filename);
    return file2string(t);
}

bool string2file(
    const std::string &filename,
    const std::string &value
)
{
    FILE* f = fopen(filename.c_str(), "w");
    if (!f)
        return false;
    fwrite(value.c_str(), value.size(), 1, f);
    fclose(f);
    return true;
}

/**
 * @return  LoRa datarate identifier e.g. "SF7BW125"
 */
std::string DATA_RATE2string(
    const DATA_RATE &value
)
{
    int bandwithValue;
    switch (value.bandwidth) {
        case BANDWIDTH_INDEX_7KHZ:
            bandwithValue = 7; // 7.8
            break;
        case BANDWIDTH_INDEX_10KHZ:
            bandwithValue = 10; // 10.4
            break;
        case BANDWIDTH_INDEX_15KHZ:
            bandwithValue = 15; // 15.6
            break;
        case BANDWIDTH_INDEX_20KHZ:
            bandwithValue = 20; // 20.8
            break;
        case BANDWIDTH_INDEX_31KHZ:
            bandwithValue = 31; // 31.2
            break;
        case BANDWIDTH_INDEX_41KHZ:
            bandwithValue = 41; // 41.6
            break;
        case BANDWIDTH_INDEX_62KHZ:
            bandwithValue = 62; // 62.5
            break;
        case BANDWIDTH_INDEX_125KHZ:
            bandwithValue = 125; // 125
            break;
        case BANDWIDTH_INDEX_250KHZ:
            bandwithValue = 250; // 250
            break;
        case BANDWIDTH_INDEX_500KHZ:
            bandwithValue = 500; // 500
            break;
        default:
            bandwithValue = 250;
            break;
    }
    std::stringstream ss;
    // e.g. SF7BW203
    ss << "SF" << (int) value.spreadingFactor
       << "BW"  << bandwithValue;
    return ss.str();
}

std::string DATA_RATE2string(
    BANDWIDTH bandwidth,
    SPREADING_FACTOR spreadingFactor
)
{
    int bandwithValue;
    switch (bandwidth) {
        case BANDWIDTH_INDEX_7KHZ:
            bandwithValue = 7; // 7.8
            break;
        case BANDWIDTH_INDEX_10KHZ:
            bandwithValue = 10; // 10.4
            break;
        case BANDWIDTH_INDEX_15KHZ:
            bandwithValue = 15; // 15.6
            break;
        case BANDWIDTH_INDEX_20KHZ:
            bandwithValue = 20; // 20.8
            break;
        case BANDWIDTH_INDEX_31KHZ:
            bandwithValue = 31; // 31.2
            break;
        case BANDWIDTH_INDEX_41KHZ:
            bandwithValue = 41; // 41.6
            break;
        case BANDWIDTH_INDEX_62KHZ:
            bandwithValue = 62; // 62.5
            break;
        case BANDWIDTH_INDEX_125KHZ:
            bandwithValue = 125; // 125
            break;
        case BANDWIDTH_INDEX_250KHZ:
            bandwithValue = 250; // 250
            break;
        case BANDWIDTH_INDEX_500KHZ:
            bandwithValue = 500; // 500
            break;
        default:
            bandwithValue = 250;
            break;
    }
    std::stringstream ss;
    // e.g. SF7BW203
    ss << "SF" << (int) spreadingFactor
       << "BW"  << bandwithValue;
    return ss.str();
}

/**
 * @param retVal return value
 * @param value LoRa data rate identifier e.g. "SF7BW125"
 */
void string2DATA_RATE(
    DATA_RATE &retVal,
    const std::string &value
)
{
    size_t sz = value.size();
    if (sz < 3)
        return;
    std::size_t p = value.find('B');
    if (p == std::string::npos)
        return;
    std::string s = value.substr(2, p - 2);
    retVal.spreadingFactor = static_cast<SPREADING_FACTOR>(atoi(s.c_str()));
    s = value.substr(p + 2);
    int bandwithValue = atoi(s.c_str());
    switch (bandwithValue) {
        case 7:
            retVal.bandwidth = BANDWIDTH_INDEX_7KHZ; // 7.8
            break;
        case 10:
            retVal.bandwidth = BANDWIDTH_INDEX_10KHZ; // 10.4
            break;
        case 15:
            retVal.bandwidth = BANDWIDTH_INDEX_15KHZ; // 15.6
            break;
        case 20:
            retVal.bandwidth = BANDWIDTH_INDEX_20KHZ; // 20.8
            break;
        case 31:
            retVal.bandwidth = BANDWIDTH_INDEX_31KHZ; // 31.2
            break;
        case 41:
            retVal.bandwidth = BANDWIDTH_INDEX_41KHZ; // 41.6
            break;
        case 62:
            retVal.bandwidth = BANDWIDTH_INDEX_62KHZ; // 62.5
            break;
        case 125:
            retVal.bandwidth = BANDWIDTH_INDEX_125KHZ; // 125
            break;
        case 250:
            retVal.bandwidth = BANDWIDTH_INDEX_250KHZ;
            break;
        case 500:
            retVal.bandwidth = BANDWIDTH_INDEX_500KHZ;
            break;
        default:
            retVal.bandwidth = BANDWIDTH_INDEX_250KHZ;
            break;
    }
}

#define NIP_COUNT 14
static const char *NETWORK_IDENTITY_PROPERTY_NAMES[NIP_COUNT] {
    "",
    "addr",           ///< address
    "activation",     ///< activation type: ABP or OTAA
    "class",          ///< A, B, C
    "deveui",	      ///< device identifier 8 bytes (ABP device may not store EUI)
    "nwkskey",		  ///< shared session key 16 bytes
    "appskey",        ///< private key 16 bytes
    "version",
    // OTAA
    "appeui",		  ///< OTAA application identifier
    "appkey",		  ///< OTAA application private key
    "nwkkey",        ///< OTAA network key
    "devnonce",      ///< last device nonce
    "joinnonce",     ///< last Join nonce
    // added for searching
    "name"
};

const char *NETWORK_IDENTITY_PROPERTY2string(
    NETWORK_IDENTITY_PROPERTY p
)
{
    if ((int) p >= NIP_COUNT || (int) p < 0)
        p = NIP_NONE;
    return NETWORK_IDENTITY_PROPERTY_NAMES[(int) p];
}

#define NETWORK_IDENTITY_COMPARISON_OPERATOR_STRING_SIZE    7
const char *NETWORK_IDENTITY_COMPARISON_OPERATOR_STRING [NETWORK_IDENTITY_COMPARISON_OPERATOR_STRING_SIZE] {
    "",
    "=",
    "<>",
    ">",
    "<",
    ">=",
    "<="
};

#define NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR_STRING_SIZE    3
const char *NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR_STRING [NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR_STRING_SIZE] {
    "",
    "and",
    "or"
};

static const char *NETWORK_IDENTITY_COMPARISON_OPERATOR2string(
    NETWORK_IDENTITY_COMPARISON_OPERATOR value
)
{
    if ((int) value >= NETWORK_IDENTITY_COMPARISON_OPERATOR_STRING_SIZE)
       value = NICO_NONE;
    return NETWORK_IDENTITY_COMPARISON_OPERATOR_STRING[value];
}

static NETWORK_IDENTITY_COMPARISON_OPERATOR stringNETWORK_IDENTITY_COMPARISON_OPERATOR(
    const char *value
)
{
    for (int a = 0; a < NETWORK_IDENTITY_COMPARISON_OPERATOR_STRING_SIZE; a++)
    {
        if (strcmp(value, NETWORK_IDENTITY_COMPARISON_OPERATOR_STRING[a]) == 0)
            return (NETWORK_IDENTITY_COMPARISON_OPERATOR) a;

    }
    return NICO_NONE;
}

static const char *NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR2string(
    NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR value
)
{
    if ((int) value >= NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR_STRING_SIZE)
        value = NILPO_NONE;
    return NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR_STRING[value];
}

static NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR string2NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR(
    const char *value
)
{
    for (int a = 0; a < NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR_STRING_SIZE; a++)
    {
        if (strcmp(value, NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR_STRING[a]) == 0)
            return (NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR) a;

    }
    return NILPO_NONE;
}

NETWORK_IDENTITY_PROPERTY string2NETWORK_IDENTITY_PROPERTY(
    const char *value
)
{
    auto f = std::find_if(NETWORK_IDENTITY_PROPERTY_NAMES, NETWORK_IDENTITY_PROPERTY_NAMES + NIP_COUNT,
      [value](const char *v) {
          return strcmp(value, v) == 0;
    });
    if (f == NETWORK_IDENTITY_PROPERTY_NAMES + NIP_COUNT)
        return NIP_NONE;
    return (NETWORK_IDENTITY_PROPERTY) (f - NETWORK_IDENTITY_PROPERTY_NAMES);
}

static std::string filterValue2string(
    const NETWORK_IDENTITY_FILTER &filter
)
{
    std::string r;
    switch (filter.property) {
        case NIP_ADDRESS:
            r = DEVADDR2string(*(DEVADDR*) &filter.filterData);
            break;
        case NIP_ACTIVATION:
            r = activation2string(*(ACTIVATION *) &filter.filterData);
            break;
        case NIP_DEVICE_CLASS:
            r = deviceclass2string(*(DEVICECLASS *) &filter.filterData);
            break;
        case NIP_DEVEUI:
        case NIP_APPEUI:
            r = DEVEUI2string(*(DEVEUI *) &filter.filterData);
            break;
        case NIP_NWKSKEY:
        case NIP_APPSKEY:
        case NIP_APPKEY:
        case NIP_NWKKEY:
            r = KEY2string(*(KEY128 *) &filter.filterData);
            break;
        case NIP_LORAWAN_VERSION:
            r = LORAWAN_VERSION2string(*(LORAWAN_VERSION *) &filter.filterData);
            break;
        case NIP_DEVNONCE:
            r = DEVNONCE2string(*(DEVNONCE *) &filter.filterData);
            break;
        case NIP_JOINNONCE:
            r = JOINNONCE2string(*(JOINNONCE *) &filter.filterData);
            break;
        case NIP_DEVICENAME:
            r = DEVICENAME2string(*(DEVICENAME *) &filter.filterData);
            break;
        default:
            break;
    }
    return r;
}

std::string NETWORK_IDENTITY_FILTER2string(
    const NETWORK_IDENTITY_FILTER &filter,
    bool isFirst
)
{
    std::stringstream ss;
    if (!isFirst)
        ss << NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR2string(filter.pre) << ' ';
    ss << NETWORK_IDENTITY_PROPERTY2string(filter.property)
        << ' ' << NETWORK_IDENTITY_COMPARISON_OPERATOR2string(filter.comparisonOperator)
        << " '" << filterValue2string(filter) << "'";
    // compareWith.
    return ss.str();
}

std::string NETWORK_IDENTITY_FILTERS2string(
    const std::vector<NETWORK_IDENTITY_FILTER> &filters
) {
    bool isFirst = true;
    std::stringstream statement;
    for (auto &f: filters) {
        statement << NETWORK_IDENTITY_FILTER2string(f, isFirst) << ' ';
        isFirst = false;
    }
    return statement.str();
}

enum IdentityFiltersParseState
{
    IFPS_PROPERTY,
    IFPS_COMPARE,
    IFPS_VALUE,
    IFPS_AND_OR
};

static void stripQuotes(
    std::string &quotedString
)
{
    if (quotedString.empty())
        return;
    if (quotedString[0] == '"' || quotedString[0] == '\'')
        quotedString = quotedString.substr(1);
    size_t l = quotedString.size();
    if (quotedString[l - 1] == '"' || quotedString[l - 1] == '\'')
        quotedString = quotedString.substr(0, l - 1);
}

int string2NETWORK_IDENTITY_FILTERS(
    std::vector <NETWORK_IDENTITY_FILTER> &retVal,
    const char *expression,
    size_t size
)
{
    size_t start = 0;
    size_t eolp = size;
    size_t finish;

    NETWORK_IDENTITY_FILTER f {NILPO_AND, NIP_NONE, NICO_NONE, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
    IdentityFiltersParseState state = IFPS_PROPERTY;
    while (start < size) {
        // skip spaces if exists
        for (auto p = start; p < eolp; p++) {
            if (!std::isspace(expression[p])) {
                start = p;
                break;
            }
        }

        // read token
        finish = eolp;
        for (auto p = start; p < eolp; p++) {
            if (isspace(expression[p])) {
                finish = p;
                break;
            }
        }

        std::string token(expression + start, finish - start);

        if (token.empty())
            break;
        switch (state) {
        case IFPS_PROPERTY:
            f.property = string2NETWORK_IDENTITY_PROPERTY(token.c_str());
            if (f.property == NIP_NONE)
                break;
            state = IFPS_COMPARE;
            break;
        case IFPS_COMPARE:
            f.comparisonOperator = stringNETWORK_IDENTITY_COMPARISON_OPERATOR(token.c_str());
            state = IFPS_VALUE;
            break;
        case IFPS_VALUE:
            stripQuotes(token);
            switch (f.property) {
                case NIP_ADDRESS:
                    string2DEVADDR((DEVADDR &) f.filterData, token);
                    f.length = sizeof(DEVADDR);
                    break;
                case NIP_ACTIVATION:
                    *(ACTIVATION *) f.filterData = string2activation(token);
                    f.length = sizeof(ACTIVATION);
                    break;
                case NIP_DEVICE_CLASS:
                    *(DEVICECLASS *) f.filterData = string2deviceclass(token);
                    f.length = sizeof(DEVICECLASS);
                    break;
                case NIP_DEVEUI:
                case NIP_APPEUI:
                    string2DEVEUI((DEVEUI &) f.filterData, token);
                    f.length = sizeof(DEVEUI);
                    break;
                case NIP_NWKSKEY:
                case NIP_APPSKEY:
                case NIP_APPKEY:
                case NIP_NWKKEY:
                    string2KEY((KEY128 &) f.filterData, token);
                    f.length = sizeof(KEY128);
                    break;
                case NIP_LORAWAN_VERSION:
                    *(LORAWAN_VERSION *) f.filterData = string2LORAWAN_VERSION(token);
                    f.length = sizeof(LORAWAN_VERSION);
                    break;
                case NIP_DEVNONCE:
                    (DEVNONCE) f.filterData = string2DEVNONCE(token);
                    f.length = sizeof(DEVNONCE);
                    break;
                case NIP_JOINNONCE:
                    string2JOINNONCE((JOINNONCE &) f.filterData, token);
                    f.length = sizeof(JOINNONCE);
                    break;
                case NIP_DEVICENAME:
                    string2DEVICENAME((DEVICENAME &) f.filterData, token.c_str());
                    f.length = (uint8_t) (token.size() < sizeof(DEVICENAME) ? token.size() : sizeof(DEVICENAME));
                    break;
                default:
                    break;
            }
            retVal.push_back(f);
            f.property = NIP_NONE;
            state = IFPS_AND_OR;
            break;
        case IFPS_AND_OR:
            f.pre = string2NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR(token.c_str());
            if (f.pre == NILPO_NONE)
                break;
            state = IFPS_PROPERTY;
            break;
        default:
            break;
        }
        start = finish;
    }
    return 0;
}
