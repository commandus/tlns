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
        uint16_t fcnt;	// frame counter 0..65535
        uint8_t fopts_fport_payload[255];  // 0..15, 1, 255 bytes

        bool operator==(const DOWNLINK_STORAGE &rhs) const;
        // variable fields accessors
        const uint8_t* fopts() const;
        const std::string foptsString() const;
        uint8_t foptsSize() const;
        void setFopts(uint8_t* value, uint8_t size);
        uint8_t fport() const;
        void setFport(uint8_t value);
        const uint8_t* payload() const;
        const std::string payloadString() const;
        void setPayload(uint8_t* value, uint8_t size);
        void setFOpts(void* value, size_t size);
}
);                                  // 4 1 2 255 =  262 bytes

#define FOPTS_FPORT_PAYLOAD_SIZE    255
#define SIZE_DOWNLINK_EMPTY_STORAGE 7
#define SIZE_DOWNLINK_STORAGE       262

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
        uint16_t fcnt;	// frame counter 0..65535
        uint8_t fopts_fport_payload[FOPTS_FPORT_PAYLOAD_SIZE];  // 0..15, 1, 255 bytes
        bool operator==(const UPLINK_STORAGE &rhs) const;

        // variable fields accessors
        const uint8_t* fopts() const;
        const std::string foptsString() const;
        uint8_t foptsSize() const;
        void setFopts(uint8_t* value, uint8_t size);
        uint8_t fport() const;
        void setFport(uint8_t value);
        const uint8_t* payload() const;
        const std::string payloadString() const;
        void setPayload(uint8_t* value, uint8_t size);
        void setFOpts(void* value, size_t size);
    }
);                                  // 4 1 2 255 =  262 bytes

#define SIZE_UPLINK_EMPTY_STORAGE 7
#define SIZE_UPLINK_STORAGE 262

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
        int payloadSize;
        LORAWAN_MESSAGE_STORAGE();
        LORAWAN_MESSAGE_STORAGE(const LORAWAN_MESSAGE_STORAGE& value);
        explicit LORAWAN_MESSAGE_STORAGE(const std::string &base64string);
        std::string toString() const;

        size_t toArray(void *buf, size_t size, const NetworkIdentity *aIdentity) const;
        // decode message
        void decode(const NetworkIdentity *aIdentity);
        void decode(const DEVADDR &devAddr, const KEY128 &appSKey);

        const DEVADDR* getAddr() const;
        const JOIN_REQUEST_FRAME *getJoinRequest() const;
        bool operator==(const LORAWAN_MESSAGE_STORAGE &rhs) const;
        LORAWAN_MESSAGE_STORAGE& operator=(const LORAWAN_MESSAGE_STORAGE &value);
        void setPayload(void* value, size_t size);
        const std::string foptsString() const;
        void setFOpts(void* value, size_t size);
        std::string payloadBase64() const;
        std::string payloadString() const;
        void setSize(size_t size);
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

bool base64SetToLORAWAN_MESSAGE_STORAGE(
    LORAWAN_MESSAGE_STORAGE &retVal,
    const std::string &base64string
);

bool hexSetToLORAWAN_MESSAGE_STORAGE(
    LORAWAN_MESSAGE_STORAGE &retVal,
    const std::string &hexString
);

#endif
