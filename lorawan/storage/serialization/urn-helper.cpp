#include <sstream>
#include <iomanip>
#include <cstring>
#include "lorawan/storage/serialization/urn-helper.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/helper/crc-helper.h"

const char* URN_PREFIX = "LW:";
const char* SCHEMA_ID = "D0:";
const char* DLMT = ":";

static uint16_t calcCheckSum(
    const std::string &urn
)
{
    return crc16modbus((uint8_t *) urn.c_str(), urn.size());
}

LorawanIdentificationURN::LorawanIdentificationURN()
    : crc(0xffff), command('\0'), offset(0), size(0)
{

}

LorawanIdentificationURN::LorawanIdentificationURN(
    const std::string &urn
)
    : crc(0xffff), command('\0'), offset(0), size(0)
{
    parse(urn);
}

bool LorawanIdentificationURN::parseToken(
    const std::string &token,
    int &count
)
{
    switch (count) {
        case 0:
            if (token != "LW")
                return false;
            break;
        case 1:
            if (token != "D0")
                return false;
            break;
        case 2:
            string2DEVEUI(networkIdentity.value.devid.id.appEUI, token);
            break;
        case 3:
            string2DEVEUI(networkIdentity.value.devid.id.devEUI, token);
            break;
        default:
            // optional
        {
            if (token.empty())
                return false;
            switch (token[0]) {
                case 'C':
                    crc = (uint16_t) strtoul(token.c_str() + 1, nullptr, 16);
                    break;
                case 'O':
                    ownerToken = token.substr(1);
                case 'S':
                    serialNumber = token.substr(1);
                    break;
                case 'P':
                    if (token.size() < 2)
                        return false;
                    switch (token[1]) {
                        case 'D':    // device address
                            string2DEVADDR(networkIdentity.value.devaddr, token.substr(2));
                            break;
                        case 'T':    // activation
                            networkIdentity.value.devid.id.activation = string2activation(token.substr(2));
                            break;
                        case 'C':     // device class
                            networkIdentity.value.devid.setClass(string2deviceclass(token.substr(2)));
                            break;
                        case 'W':     // nwkSKey
                            string2KEY(networkIdentity.value.devid.id.nwkSKey, token.substr(2));
                            break;
                        case 'S':    // appSKey
                            string2KEY(networkIdentity.value.devid.id.appSKey, token.substr(2));
                            break;
                        case 'V':    // LoRaWAN version
                            networkIdentity.value.devid.id.version = string2LORAWAN_VERSION(token.substr(2));
                            break;
                        case 'A':    // appKey
                            string2KEY(networkIdentity.value.devid.id.appKey, token.substr(2));
                            break;
                        case 'N':    // nwkKey
                            string2KEY(networkIdentity.value.devid.id.nwkKey, token.substr(2));
                            break;
                        case 'O':    // devNonce
                            networkIdentity.value.devid.id.devNonce = string2DEVNONCE(token.substr(2));
                            break;
                        case 'J':    // joinNonce
                            string2JOINNONCE(networkIdentity.value.devid.id.joinNonce, token.substr(2));
                            break;
                        case 'X':     // 'command': 'A', 'I', 'L', 'C', 'N', 'P', 'R', 'S', 'E'
                            if (token.size() > 1)
                                command = token[2];
                            break;
                        case 'F':     // offset
                            offset = (uint8_t) strtoul(token.substr(2).c_str(), nullptr, 16);
                            break;
                        case 'Z':     // size
                            size = (uint32_t) strtoul(token.substr(2).c_str(), nullptr, 16);
                            break;
                        default:
                            return false;
                    }
                    break;
                default:
                    return false;  // invalid character
            }
        }
    }
    count++;
    return true;
}

bool LorawanIdentificationURN::parse(
    const std::string &urn
)
{
    size_t p = 0;
    size_t lastP = 0;
    int count = 0;
    while ((p = urn.find(':', p)) != std::string::npos)  {
        if (!parseToken(urn.substr(lastP, p - lastP), count))
            return false;
        p++;
        lastP = p;
    }
    auto sz = urn.size();
    if (lastP < sz)
        return parseToken(urn.substr(lastP, sz - lastP), count);
    return true;
}

std::string LorawanIdentificationURN::toString() const
{
    return NETWORKIDENTITY2URN(networkIdentity, ownerToken, serialNumber, true, false, nullptr);
}

std::string mkURN(
    const DEVEUI &appEui,
    const DEVEUI &devEui,
    const PROFILEID &profileId,
    const std::string &ownerToken,
    const std::string &serialNumber,
    const std::vector<std::string> *extraProprietary,
    bool addCheckSum
)
{
    std::stringstream ss;
    // first mandatory fields
    ss << URN_PREFIX << SCHEMA_ID
       << DEVEUI2string(appEui) << DLMT
       << DEVEUI2string(devEui) << DLMT;
    // profile id from the name if it is hex number
    ss << std::hex << std::setw(8) << std::setfill('0') << profileId.u;
    if (!ownerToken.empty())
        ss << ":O" << ownerToken;
    if (!serialNumber.empty())
        ss << ":S" << serialNumber;
    if (extraProprietary) {
        for (auto &p : *extraProprietary) {
            ss << ":P" << p;
        }
    }
    std::string r = toUpperCase(ss.str());
    if (addCheckSum) {
        ss << ":C" << std::hex << std::setw(4) << std::setfill('0') << calcCheckSum(r);
        r = toUpperCase(ss.str());
    }
    return r;
}

std::string NETWORKIDENTITY2URN(
    const NETWORKIDENTITY &networkIdentity,
    const std::string &ownerToken,
    const std::string &serialNumber,
    bool addProprietary,
    bool addCheckSum,
    const std::vector<std::string> *extraProprietary
)
{
    PROFILEID pid(DEVICENAME2string(networkIdentity.value.devid.id.name));
    std::vector<std::string> proprietary;
    if (extraProprietary) {
        proprietary = *extraProprietary;
    }
    if (addProprietary) {
        proprietary.push_back("D" + DEVADDR2string(networkIdentity.value.devaddr));
        proprietary.push_back("T" + activation2string(networkIdentity.value.devid.id.activation));
        proprietary.push_back("C" + deviceclass2string(networkIdentity.value.devid.id.deviceclass));
        proprietary.push_back("W" + KEY2string(networkIdentity.value.devid.id.nwkSKey));
        proprietary.push_back("S" + KEY2string(networkIdentity.value.devid.id.appSKey));
        proprietary.push_back("V" + LORAWAN_VERSION2string(networkIdentity.value.devid.id.version));
        proprietary.push_back("A" + KEY2string(networkIdentity.value.devid.id.appKey));
        proprietary.push_back("N" + KEY2string(networkIdentity.value.devid.id.nwkKey));
        proprietary.push_back("O" + DEVNONCE2string(networkIdentity.value.devid.id.devNonce));
        proprietary.push_back("J" + JOINNONCE2string(networkIdentity.value.devid.id.joinNonce));
    }

    return mkURN(networkIdentity.value.devid.id.appEUI, networkIdentity.value.devid.id.devEUI, pid,
          ownerToken, serialNumber, &proprietary, addCheckSum);
}

size_t returnStr(
    unsigned char* retBuf,
    size_t retSize,
    const std::string &value,
    int errCode
)
{
    auto r = value.size();
    if (r <= retSize) {
        memmove(retBuf, value.c_str(), value.size());
    } else
        r = 0;
    return r;
}

size_t returnURN(
    unsigned char* retBuf,
    size_t retSize,
    const LorawanIdentificationURN &value,
    int errCode
)
{
    auto s = value.toString();
    auto r = s.size();
    if (r <= retSize) {
        memmove(retBuf, s.c_str(), s.size());
    } else
        r = 0;
    return r;
}

std::string stripURNProprietary(
    const std::string &urn
)
{
    std::stringstream ss;
    size_t p = 0;
    size_t lastP = 0;
    int count = 0;
    while ((p = urn.find(':', p)) != std::string::npos)  {
        auto c = urn.substr(lastP, p - lastP + 1);
        if (!(c.length() >= 1 && c[0] == 'P'))
            ss << c;
        p++;
        lastP = p;
    }
    auto sz = urn.size();
    if (lastP < sz) {
        auto c = urn.substr(lastP, sz - lastP + 1);
        if (!(c.length() >= 1 && c[0] == 'P'))
            ss << c;
    }
    return ss.str();
}
