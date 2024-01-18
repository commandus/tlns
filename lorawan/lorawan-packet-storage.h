#ifndef LORAWAN_PACKET_STORAGE_H_
#define LORAWAN_PACKET_STORAGE_H_

#include "lorawan-types.h"

PACK(
    class JOIN_REQUEST_STORAGE {
    public:
        uint8_t reserved;
    }
);

PACK(
    class JOIN_RESPONSE_STORAGE {
    public:
        uint8_t reserved;
    }
);

PACK(
    class DOWNLINK_STORAGE {
    public:
        DEVADDR devaddr;			// MAC address
        struct {
            uint8_t foptslen: 4;
            uint8_t fpending: 1;
            uint8_t ack: 1;
            uint8_t rfu: 1;
            uint8_t adr: 1;
        } f;
        uint8_t optsNpayload[255];
    }
);

PACK(
    class UPLINK_STORAGE {
    public:
        DEVADDR devaddr;			// MAC address
        struct {
            uint8_t foptslen: 4;
            uint8_t classb: 1;
            uint8_t ack: 1;
            uint8_t addrackreq: 1;
            uint8_t adr: 1;
        } f;
        uint8_t optsNpayload[255];
    }
);

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
            // Join request
            JOIN_REQUEST_STORAGE joinRequest;
            // Join response
            JOIN_RESPONSE_STORAGE joinResponse;
            // data downlink
            DOWNLINK_STORAGE downlink;
            // data uplink
            UPLINK_STORAGE uplink;
            // proprietary radio
            PROPRIETARY_STORAGE proprietary;
        }) data;
    }
);			// 8 bytes

PACK( class LorawanPacketStorage {
public:
    LORAWAN_MESSAGE_STORAGE msg;
    LorawanPacketStorage();
    std::string toString() const;
    const DEVADDR* getAddr() const;
} );

#endif
