#ifndef LORAWAN_PACKET_STORAGE_H_
#define LORAWAN_PACKET_STORAGE_H_

#include "lorawan/lorawan-types.h"
#include "lorawan/storage/network-identity.h"

PACK(
    class DOWNLINK_STORAGE {
    public:
        DEVADDR devaddr;			// MAC address  4 bytes
        struct {
            uint8_t foptslen: 4;
            uint8_t fpending: 1;
            uint8_t ack: 1;
            uint8_t rfu: 1;
            uint8_t adr: 1;
        } f;                        // 1 byte
        uint8_t optsNpayload[255];  // 255 bytes
        bool operator==(const DOWNLINK_STORAGE &rhs) const;
    }
);                                  // 4 1 255 =  260 bytes

#define SIZE_DOWNLINK_EMPTY_STORAGE 5
#define SIZE_DOWNLINK_STORAGE 260

PACK(
    class UPLINK_STORAGE {
    public:
        DEVADDR devaddr;			// MAC address 4 bytes
        struct {
            uint8_t foptslen: 4;
            uint8_t classb: 1;
            uint8_t ack: 1;
            uint8_t addrackreq: 1;
            uint8_t adr: 1;
        } f;                        // 1 byte
        uint8_t optsNpayload[255];  // 255 bytes
        bool operator==(const UPLINK_STORAGE &rhs) const;
    }
);                                  // 4 1 255 = 260

#define SIZE_UPLINK_EMPTY_STORAGE 5
#define SIZE_UPLINK_STORAGE 260

PACK(
    class PROPRIETARY_STORAGE {
    public:
        uint8_t optsNpayload[255];
    }
);

PACK(
    class LORAWAN_MESSAGE_STORAGE {
    public:
        MHDR mhdr;
        PACK( union {
            // as array
            uint8_t u[1];
            // Join request
            JOIN_REQUEST_FRAME joinRequest;
            // Join response
            JOIN_ACCEPT_FRAME joinResponse;
            // data downlink
            DOWNLINK_STORAGE downlink;
            // data uplink
            UPLINK_STORAGE uplink;
            // proprietary radio
            PROPRIETARY_STORAGE proprietary;
        } ) data;
        /**
         * payload size
         */
        uint16_t packetSize;
        LORAWAN_MESSAGE_STORAGE();
        LORAWAN_MESSAGE_STORAGE(const LORAWAN_MESSAGE_STORAGE& value);
        explicit LORAWAN_MESSAGE_STORAGE(const std::string &base64string);
        std::string toString() const;

        size_t toArray(void *buf, size_t size, const NetworkIdentity *aIdentity) const;
        // decode message
        void decode(const NetworkIdentity *aIdentity);

        const DEVADDR* getAddr() const;
        const JOIN_REQUEST_FRAME *getJoinRequest() const;
        bool operator==(const LORAWAN_MESSAGE_STORAGE &rhs) const;
        LORAWAN_MESSAGE_STORAGE& operator=(const LORAWAN_MESSAGE_STORAGE &value);
        std::string payloadBase64() const;
    }
);

void  setLORAWAN_MESSAGE_STORAGE(
    LORAWAN_MESSAGE_STORAGE &retVal,
    const std::string &bin
);

void setLORAWAN_MESSAGE_STORAGE(
    LORAWAN_MESSAGE_STORAGE &retVal,
    void *buffer,
    size_t size
);

bool decodeBase64ToLORAWAN_MESSAGE_STORAGE(
    LORAWAN_MESSAGE_STORAGE &retVal,
    const std::string &base64string
);

#endif
