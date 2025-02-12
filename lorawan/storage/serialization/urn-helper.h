#ifndef LORAWAN_STORAGE_URN_HELPER_H
#define LORAWAN_STORAGE_URN_HELPER_H

/**
 * TR005 LoRaWAN Device Identification QR Codes
 * @see https://resources.lora-alliance.org/document/tr005-lorawan-device-identification-qr-codes
 */
#include <string>
#include <vector>
#include "lorawan/lorawan-types.h"

typedef enum ProprietaryXCommand {
    PROPRIETARY_X_COMMAND_NONE = '\0',
    PROPRIETARY_X_COMMAND_IDENTITY_ADDR = 'A',
    PROPRIETARY_X_COMMAND_IDENTITY_EUI = 'I',
    PROPRIETARY_X_COMMAND_IDENTITY_LIST = 'L',
    PROPRIETARY_X_COMMAND_IDENTITY_COUNT = 'C',
    PROPRIETARY_X_COMMAND_IDENTITY_NEXT = 'N',
    PROPRIETARY_X_COMMAND_IDENTITY_ASSIGN = 'P',
    PROPRIETARY_X_COMMAND_IDENTITY_RM = 'R',
    PROPRIETARY_X_COMMAND_IDENTITY_FORCE_SAVE = 'S',
    PROPRIETARY_X_COMMAND_IDENTITY_CLOSE_RESOURCES = 'E'
} ProprietaryXCommand;

class LorawanIdentificationURN {
protected:
    bool parseToken(
        const std::string &token,
        int &count
    );
    bool parse(
        const std::string &urn
    );
public:
    NETWORKIDENTITY networkIdentity;
    PROFILEID profileId;
    uint16_t crc;
    std::string ownerToken;
    std::string serialNumber;
    char command;
    uint32_t offset;    // for list command
    uint8_t size;       // for list command
    LorawanIdentificationURN();
    LorawanIdentificationURN(const std::string &urn);
    std::string toString() const;
};

std::string mkURN(
    const DEVEUI &appEui,
    const DEVEUI &devEui,
    const PROFILEID &profileId,
    const std::string &ownerToken = "",
    const std::string &serialNumber = "",
    const std::vector<std::string> *extraProprietary = nullptr,
    bool addCheckSum = false
);

std::string NETWORKIDENTITY2URN(
    const NETWORKIDENTITY &networkIdentity,
    const std::string &ownerToken = "",
    const std::string &serialNumber = "",
    bool addProprietary = false,
    bool addCheckSum = false,
    const std::vector<std::string> *extraProprietary = nullptr
);

size_t returnStr(
    unsigned char* retBuf,
    size_t retSize,
    const std::string &value,
    int errCode
);

size_t returnURN(
    unsigned char* retBuf,
    size_t retSize,
    const LorawanIdentificationURN &value,
    int errCode
);

/**
 * Remove any proprietary properties from the URN (":P")
 * @param value
 * @return URN w/o proprietary properties
 */
std::string stripURNProprietary(
    const std::string &value
);

#endif
