#include "lorawan-types.h"

typedef PACK( struct {
    MHDR mhdr;
	union {
        // Join request
        struct {
        } joinRequest;
        // Join response
		struct {
        } joinResponse;
        // data downlink
		struct {
	        DEVADDR devaddr;			// MAC address
		    struct {
                uint8_t foptslen: 4;
                uint8_t fpending: 1;
                uint8_t ack: 1;
                uint8_t rfu: 1;
                uint8_t adr: 1;
		    } f;
            uint8_t optsNpayload[255];
		} downlink;
        // data uplink
		struct {
	        DEVADDR devaddr;			// MAC address
		    struct {
                uint8_t foptslen: 4;
                uint8_t classb: 1;
                uint8_t ack: 1;
                uint8_t addrackreq: 1;
                uint8_t adr: 1;
		    } f;
            uint8_t optsNpayload[255];
		} uplink;
        // proprietary radio
		struct {
            uint8_t optsNpayload[255];
		} proprietary;
	} data;
} ) LORAWAN_MESSAGE_STORAGE;			// 8 bytes

PACK( class LorawanPacketStorage {
public:
    LORAWAN_MESSAGE_STORAGE msg;
    LorawanPacketStorage();
} );
