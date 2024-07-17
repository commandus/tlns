#ifndef LORAWAN_NETWORK_TYPES_H_
#define LORAWAN_NETWORK_TYPES_H_	1

// #define PACK( __Declaration__ ) __Declaration__
#ifdef __GNUC__
    #define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#else
    #ifdef _MSC_VER
        #define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
    #else
        #define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
    #endif
#endif

#define LORAWAN_MAJOR_VERSION   0

#include <string>
#include <map>
#include <ctime>
#include <cinttypes>

#include "lorawan-const.h"

#define INVALID_ID 0xffffffff

// typedef unsigned char NETID[3];
PACK(class NETID {
private:
    int getTypeMask() const;
public:
	union {
		unsigned char c[3];
	};
    NETID();
    NETID(const NETID &value);
    NETID(uint32_t value);
    NETID(uint8_t netType, uint32_t value);
    uint8_t getType() const;
    void get(NETID &retval)  const;
    uint32_t get() const;
    NETID *getPtr() const;
    uint32_t getNetId() const;
    uint32_t getNwkId() const;
    std::string toString() const;

    void setType(uint8_t value);
    void set(const std::string &value);
    void set(const NETID &value);
    void set(uint32_t value);
    int set(uint8_t netType, uint32_t value);

    /**
     * Invalidate NetId, set RFU to zeroes
     */
    void applyTypeMask();

    int getRFUBitsCount() const;
    int getNetIdBitsCount() const;
    size_t size();
});

typedef PACK(struct {
    uint8_t v0;
    uint8_t v1;
    uint8_t v2: 5;
    uint8_t networkType: 3;	// MSB network type 0..7
} ) NETID_TYPE;		// 3 bytes

#define SIZE_NETID_TYPE 3

// typedef unsigned char DEVADDR[4];

PACK(class DEVADDR {
private:
    int setNetIdType(uint8_t value);

    int setNwkId_1_0(uint8_t netIdType, uint32_t value);
    int setNwkId_1_1(uint8_t netIdType, uint32_t value);
    int setNwkId(uint8_t netIdType, uint32_t value);

    int setNwkAddr_1_0(uint8_t netIdType, uint32_t value);
    int setNwkAddr_1_1(uint8_t netIdType, uint32_t value);
    int setNwkAddr(uint8_t netIdType, uint32_t value);

    int setMaxAddress(const NETID &netId);
    int setMinAddress(const NETID &netId);
    static uint32_t getMaxNwkId(uint8_t netTypeId);
    static uint32_t getMaxNwkAddr_1_0(uint8_t netTypeId);
    static uint32_t getMaxNwkAddr_1_1(uint8_t netTypeId);
    static uint32_t getMaxNwkAddr(uint8_t netTypeId);

    uint32_t getNwkId_1_0() const;
    uint32_t getNwkId_1_1() const;
    uint32_t getNwkAddr_1_0() const;
    uint32_t getNwkAddr_1_1() const;
public:
	union {
		unsigned char c[4];
		uint32_t u;
	};
	DEVADDR();
	std::size_t operator()(const DEVADDR &value) const;
	bool operator==(const DEVADDR &rhs) const;
	bool operator<(const DEVADDR &rhs) const;
	bool operator>(const DEVADDR &rhs) const;
    bool operator<=(const DEVADDR &rhs) const;
    bool operator>=(const DEVADDR &rhs) const;
	bool operator!=(const DEVADDR &rhs) const;

	DEVADDR(const DEVADDR &value);
    DEVADDR(uint32_t value);
    DEVADDR(const std::string &value);
    DEVADDR(const NETID &netid, uint32_t nwkAddr);
    DEVADDR(uint8_t netTypeId, uint32_t nwkId, uint32_t nwkAddr);
    // min/max addr
    DEVADDR(const NETID &netId, bool retMax);
    
    void get(DEVADDR &retval) const;
    uint32_t get() const;
    std::string toString() const;

    uint8_t getNetIdType() const;
    // NwkId is a part of NetId for types 3..7
    uint32_t getNwkId() const;
    uint32_t getNwkAddr() const;

    void set(const std::string &value);
    void set(const DEVADDR &value);
    void set(uint32_t value);

    int set(uint8_t netTypeId, uint32_t nwkId, uint32_t nwkAddr);
    // Set address only (w/o nwkId)
    int setAddr(uint32_t nwkAddr);
    int set(const NETID &netid, uint32_t nwkAddr);

    bool empty() const;

    int increment();
    int decrement();
    DEVADDR& operator++();  // prefix increment operator
    DEVADDR& operator--();  // prefix decrement operator
    DEVADDR& operator=(const DEVADDR&);

    static uint8_t getTypePrefixBitsCount(uint8_t netTypeId);
    static uint8_t getNwkIdBitsCount(uint8_t typ);
    static uint8_t getNwkAddrBitsCount(uint8_t typ);

    // return dev address space size
    size_t size();
});	// 4 bytes

#define SIZE_DEVADDR 4

// typedef unsigned char KEY128[16];
PACK(class KEY128 {
public:
    union {
        unsigned char c[16];
        struct {
            uint64_t u[2];
        };
    };
    KEY128();
    explicit KEY128(const std::string &hex);
    explicit KEY128(const char* hex);
    KEY128(const KEY128 &value);
    KEY128(uint64_t hi, uint64_t lo);
    std::size_t operator()(const KEY128 &value) const;
    bool operator==(const KEY128 &rhs) const;
    bool operator<(const KEY128 &rhs) const;
    bool operator>(const KEY128 &rhs) const;
    bool operator!=(const KEY128 &rhs) const;
});   // 16 bytes

#define SIZE_KEY128 16

/*
typedef unsigned char DEVEUI[8];
*/
PACK(class DEVEUI {
public:
	union {
		unsigned char c[8];
		uint64_t u;
	};
	DEVEUI();
	explicit DEVEUI(const std::string &hex);
    explicit DEVEUI(uint64_t value);
	std::size_t operator()(const DEVEUI &value) const {
		return value.u;
	}
	bool operator==(const DEVEUI &rhs) const;
	bool operator<(const DEVEUI &rhs) const;
	bool operator>(const DEVEUI &rhs) const;
	bool operator!=(const DEVEUI &rhs) const;
});

#define SIZE_DEVEUI 9

// typedef unsigned char JOINNONCE[3];
PACK(class JOINNONCE {
public:
    union {
        unsigned char c[3];
    };
    JOINNONCE();
    explicit JOINNONCE(const std::string &hex);
    explicit JOINNONCE(uint32_t value);
});

#define SIZE_JOINNONCE 3

// typedef uint16_t DEVNONCE;
PACK(class DEVNONCE {
public:
    union {
        unsigned char c[2];
        uint16_t u;
    };
    DEVNONCE();
    explicit DEVNONCE(const std::string& hex);
    explicit DEVNONCE(uint16_t value);
});

#define SIZE_DEVNONCE 2

// typedef unsigned char APPNONCE[3];
PACK(class APPNONCE {
public:
    union {
        unsigned char c[3];
    };
    APPNONCE();
    explicit APPNONCE(const std::string& hex);
    explicit APPNONCE(uint32_t value);
});

#define SIZE_APPNONCE 3

typedef uint8_t FREQUENCY[3];

PACK(class DEVICENAME {
public:
    char c[8];
    DEVICENAME();
    explicit DEVICENAME(const std::string &value);
    explicit DEVICENAME(const char *value);
    std::string toString() const;
});

#define SIZE_DEVICENAME 8

enum ERR_CODE_TX {
	JIT_TX_OK = 0,                 	// Packet ok to be sent {"txpk_ack":{"error":"NONE"}}"
	JIT_TX_ERROR_TOO_LATE = 1,     	// Too late to send this packet {"txpk_ack":{"error":"TOO_LATE"}}
	JIT_TX_ERROR_TOO_EARLY = 2,    	// Too early to queue this packet {"txpk_ack":{"error":"TOO_EARLY"}}
	JIT_TX_ERROR_FULL = 3,         	// Downlink queue is full
	JIT_TX_ERROR_EMPTY = 4,        	// Downlink queue is empty
	JIT_TX_ERROR_COLLISION_PACKET = 5, // A packet is already enqueued for this timeframe {"txpk_ack":{"error":"COLLISION_PACKET"}}
	JIT_TX_ERROR_COLLISION_BEACON = 6, // A beacon is planned for this timeframe {"txpk_ack":{"error":"COLLISION_BEACON"}}
	JIT_TX_ERROR_TX_FREQ = 7,      	// The required frequency for downlink is not supported {"txpk_ack":{"error":"TX_FREQ"}}
	JIT_TX_ERROR_TX_POWER = 8,     	// The required power for downlink is not supported {"txpk_ack":{"error":"TX_POWER"}}
	JIT_TX_ERROR_GPS_UNLOCKED = 9, 	// GPS timestamp could not be used as GPS is unlocked {"txpk_ack":{"error":"GPS_UNLOCKED"}}
	JIT_TX_ERROR_INVALID = 10       // Packet is invalid
};

// network server receives SEMTECH_GW_PUSH_DATA, SEMTECH_GW_PULL_DATA, SEMTECH_GW_TX_ACK
// gateway forward the RF packets received, and associated metadata, to the network server
#define SEMTECH_GW_PUSH_DATA	0
// network server responds on PUSH_DATA to acknowledge immediately all the PUSH_DATA packets received
#define SEMTECH_GW_PUSH_ACK		1
// gateway initiate receiving packets from the network server (because of NAT)
#define SEMTECH_GW_PULL_DATA	2
// network server send packet to the gateway after PULL_DATA - PULL_ACK sequence
#define SEMTECH_GW_PULL_RESP	3
// network server responds on PULL_DATA
#define SEMTECH_GW_PULL_ACK		4
// gateway inform network server about does PULL_RESP data transmission was successful or not
#define SEMTECH_GW_TX_ACK		5

typedef PACK( struct {
	uint8_t version;			// protocol version = 2
	uint16_t token;				// random token
	uint8_t tag;				// PUSH_DATA 0x00 PULL_DATA 0x02
} ) SEMTECH_PREFIX;		        // 4 bytes

#define SIZE_SEMTECH_PREFIX  4

typedef PACK( struct {
    uint64_t gatewayId;
    time_t t;					// UTC time of pkt RX, us precision, ISO 8601 'compact' format
    uint32_t tmst;				// Internal timestamp of "RX finished" event (32b unsigned). In microseconds
    uint8_t chan;				// Concentrator "IF" channel used for RX (unsigned integer)
    uint8_t rfch;				// Concentrator "RF chain" used for RX (unsigned integer)
    uint32_t freq;				// RX central frequency in Hz, not Mhz. MHz (unsigned float, Hz precision) 868.900000
    int8_t stat;				// CRC status: 1 = OK, -1 = fail, 0 = no CRC
    MODULATION modu;			// MODULATION_LORA, MODULATION_FSK
    BANDWIDTH bandwidth;
    SPREADING_FACTOR spreadingFactor;
    CODING_RATE codingRate;
    uint32_t bps;				// MODULATION_FSK bits per second
    int16_t rssi;				// RSSI in dBm (signed integer, 1 dB precision) e.g. -35
    float lsnr; 				// Lora SNR ratio in dB (signed float, 0.1 dB precision) e.g. 5.1
} ) SEMTECH_PROTOCOL_METADATA_RX;

/**
 * Semtech PUSH DATA packet described in section 3.2
 * Semtech PULL DATA packet described in section 5.2
 * PUSH_DATA, PULL_DATA packets prefix.
 * @see https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT sections 3.2, 5,2
 */
typedef PACK( struct {
	uint8_t version;			// protocol version = 2
	uint16_t token;				// random token
	uint8_t tag;				// PUSH_DATA 0x00 PULL_DATA 0x02
	DEVEUI mac;					// 4-11	Gateway unique identifier (MAC address). For example : 00:0c:29:19:b2:37
} ) SEMTECH_PREFIX_GW;	        // 12 bytes

/**
@struct lgw_pkt_tx_s
@brief Structure containing the configuration of a packet to send and a pointer to the payload
*/
typedef PACK( struct {
    uint32_t    freq_hz;        ///> center frequency of
    uint8_t     tx_mode;        ///> select on what event/time the TX is triggered IMMEDIATE 0, TIMESTAMPED 1, ON_GPS 2
    uint32_t    count_us;       ///> timestamp or delay in microseconds for TX trigger
    uint8_t     rf_chain;       ///> through which RF chain will the packet be sent
    int8_t      rf_power;       ///> TX power, in dBm
    uint8_t     modulation;     ///> modulation to use for the packet
    uint8_t     bandwidth;      ///> modulation bandwidth (LoRa only)
    uint32_t    datarate;       ///> TX datarate (baudrate for MODULATION_FSK, SF for LoRa)
    uint8_t     coderate;       ///> error-correcting code of the packet (LoRa only)
    bool        invert_pol;     ///> invert signal polarity, for orthogonal downlinks (LoRa only)
    uint8_t     f_dev;          ///> frequency deviation, in kHz (MODULATION_FSK only)
    uint16_t    preamble;       ///> set the preamble length, 0 for default
    bool        no_crc;         ///> if true, do not send a CRC in the packet
    bool        no_header;      ///> if true, enable implicit header mode (LoRa), fixed length (MODULATION_FSK)
    uint16_t    size;           ///> payload size in bytes
    // uint8_t     payload[256];   ///> buffer containing the payload
} ) SEMTECH_PROTOCOL_METADATA_TX;

#define SIZE_SEMTECH_PREFIX_GW 12

/**
 * PUSH_ACK packet
 * @see https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT section 3.3
 */
typedef PACK( struct {
	uint8_t version;			// protocol version = 2
	uint16_t token;				// same random token as SEMTECH_PREFIX_GW
	uint8_t tag;				// PUSH_ACK 1 PULL_ACK 4
} ) SEMTECH_ACK;			    // 4 bytes

#define SIZE_SEMTECH_ACK 4

/**
 * 
 * MAC message types(7..5) RFU(4..2) Major(1..0)
 * Hex   Bin Hex
 * 00000000 0    Join-request
 * 00100000 20   Join-accept
 * 01000000 40   Unconfirmed Data Up
 * 01100000 60   Unconfirmed Data Down
 * 10000000 80   Confirmed Data Up
 * 10100000 A0   Confirmed Data Down
 * 11000000 C0   Rejoin-request
 * 11100000 E0   ProprietaryRADIO
*/

typedef enum {
	MTYPE_JOIN_REQUEST = 0,
 	MTYPE_JOIN_ACCEPT = 1,
 	MTYPE_UNCONFIRMED_DATA_UP = 2,      ///< sent by end-devices to the Network Server
 	MTYPE_UNCONFIRMED_DATA_DOWN = 3,    ///< sent by the Network Server to only one end-device and is relayed by a single gateway
 	MTYPE_CONFIRMED_DATA_UP = 4,
 	MTYPE_CONFIRMED_DATA_DOWN = 5,
 	MTYPE_REJOIN_REQUEST = 6,
 	MTYPE_PROPRIETARYRADIO = 7
} MTYPE;

// Join-request types.
typedef enum {
	JOINREQUEST = 0xff,
	REJOINREQUEST0 = 0,
	REJOINREQUEST1 = 1,
	REJOINREQUEST2 = 2
 } JOINREQUESTTYPE;

PACK( class MHDR {
public:
	union {
		uint8_t i;
		struct {
			uint8_t major: 2;   ///< always 0
			uint8_t rfu: 3;     ///< reserved
			uint8_t mtype: 3;   ///< enum MTYPE
		} f;
	};
    bool operator==(const MHDR &rhs) const;
    bool operator!=(const MHDR &rhs) const;
} );			// 1 byte

#define SIZE_MHDR 1

typedef PACK( struct {
    // Frame header (FHDR)
    DEVADDR devaddr;			// MAC address
    union {
        uint8_t i;
        // downlink
        struct {
            uint8_t foptslen: 4;
            uint8_t fpending: 1;
            uint8_t ack: 1;
            uint8_t rfu: 1;
            uint8_t adr: 1;
        } f;
        // uplink
        struct {
            uint8_t foptslen: 4;
            uint8_t classb: 1;
            uint8_t ack: 1;
            uint8_t addrackreq: 1;
            uint8_t adr: 1;
        } fup;
    } fctrl;	// frame control
    uint16_t fcnt;	// frame counter 0..65535
    // FOpts 0..15
} ) FHDR;			// 7+ bytes

#define SIZE_FHDR   7
#define SIZE_FPORT  1

/**
 * MHDR + FHDR
 */ 
typedef PACK( struct {
	// MAC header byte: message type, RFU, Major
	MHDR macheader;			    // 0x40 unconfirmed uplink
	// Frame header (FHDR)
	FHDR fhdr;			        // Frame header 7+
} ) RFM_HEADER;			// 8 bytes, +1

#define SIZE_RFM_HEADER 8

PACK(
class JOIN_REQUEST_FRAME {
public:
	DEVEUI joinEUI;			    // AppEUI, JoinEUI
	DEVEUI devEUI;			    // DevEUI
	DEVNONCE devNonce;
    bool operator==(const JOIN_REQUEST_FRAME &rhs) const;
    bool operator<(const JOIN_REQUEST_FRAME &rhs) const;
    bool operator>(const JOIN_REQUEST_FRAME &rhs) const;
    bool operator<=(const JOIN_REQUEST_FRAME &rhs) const;
    bool operator>=(const JOIN_REQUEST_FRAME &rhs) const;
    bool operator!=(const JOIN_REQUEST_FRAME &rhs) const;
} ) ;	// 8 + 8 + 2 = 18 bytes

#define SIZE_JOIN_REQUEST_FRAME 18

typedef PACK( struct {
    MHDR mhdr;  			    // 0x00 Join request. MAC header byte: message type, RFU, Major
    JOIN_REQUEST_FRAME frame;
    uint32_t mic;			    // MIC
} ) JOIN_REQUEST_HEADER;	// 1 + 18 + 4 = 23 bytes

#define SIZE_JOIN_REQUEST_HEADER 23
#define SIZE_MIC    4

typedef PACK( struct {
    uint8_t RX2DataRate: 4;	    ///< downlink data rate that serves to communicate with the end-device on the second receive window (RX2)
    uint8_t RX1DROffset: 3;	    ///< offset between the uplink data rate and the downlink data rate used to communicate with the end-device on the first receive window (RX1)
    uint8_t optNeg: 1;     	    ///< 1.0- RFU, 1.1- optNeg
} ) DLSETTINGS;	        // 1 byte

#define SIZE_DLSETTINGS 1
/**
 * Join-Accept
  NetID DevAddr DLSettings RXDelay CFList
 */
typedef PACK( struct {
    JOINNONCE joinNonce;   	        //
    NETID netId;   	                //
    DEVADDR devAddr;   	            //
    DLSETTINGS dlSettings;		    // downlink configuration settings
    uint8_t rxDelay;                //
} ) JOIN_ACCEPT_FRAME_HEADER;	    // 3 3 4 1 1 = 12 bytes

#define SIZE_JOIN_ACCEPT_FRAME_HEADER 12

// MHDR mhdr;  			            // 0x00 Join request. MAC header byte: message type, RFU, Major
PACK( class JOIN_ACCEPT_FRAME {
public:
    JOIN_ACCEPT_FRAME_HEADER hdr;   //
    uint32_t mic;			        // MIC
    bool operator==(const JOIN_ACCEPT_FRAME &rhs) const;
    bool operator==(const JOIN_ACCEPT_FRAME_HEADER &rhs) const;
} );	            // 12 4 = 16 bytes

#define SIZE_JOIN_ACCEPT_FRAME 16

// Channel frequency list
typedef PACK( struct {
    FREQUENCY frequency[5];	        // frequency, 100 * Hz ch 4..8
    uint8_t cflisttype;		        // always 0
} ) CFLIST;			                // 16 bytes

#define SIZE_CFLIST 16

// 0x00 Join request. MAC header byte: message type, RFU, Major
typedef PACK( struct {
    JOIN_ACCEPT_FRAME_HEADER hdr;   // 12
    CFLIST cflist;
    uint32_t mic;			        // MIC
} ) JOIN_ACCEPT_FRAME_CFLIST;	    // 12 16 4 = 32 bytes

#define SIZE_JOIN_ACCEPT_FRAME_CFLIST 32

typedef PACK( struct {
	uint8_t fopts[15];
} ) FOPTS;					    // 0..15 bytes

typedef enum {
	ABP = 0,
	OTAA = 1
} ACTIVATION;

typedef enum {
	CLASS_A = 0,
	CLASS_B = 1,
	CLASS_C = 2
} DEVICECLASS;

PACK(class LORAWAN_VERSION {
public:
    union {
        uint8_t c;
        struct {
            uint8_t major: 2;		// always 1
            uint8_t minor: 2;		// 0 or 1
            uint8_t release: 4;		// no matter
        };
    };
    LORAWAN_VERSION();
    LORAWAN_VERSION(uint8_t major, uint8_t minor, uint8_t release);
    LORAWAN_VERSION(uint8_t value);
});	// 1 byte

#define SIZE_LORAWAN_VERSION 1

// Regional paramaters version e.g. RP002-1.0.0, RP002-1.0.1
typedef PACK( struct {
    uint8_t major: 2;		// always 1
    uint8_t minor: 2;		// 0 or 1
    uint8_t release: 4;		// no matter
} ) REGIONAL_PARAMETERS_VERSION;	// 1 byte

#define SIZE_REGIONAL_PARAMETERS_VERSION 1

class NETWORKIDENTITY;
PACK(class DEVICEID {
public:
    DEVICEID(uint64_t devEUI);
    DEVICEID(const DEVEUI &devEUI);

    // value, no key
	ACTIVATION activation;	///< activation type: ABP or OTAA
	DEVICECLASS deviceclass;
	DEVEUI devEUI;		    ///< device identifier 8 bytes (ABP device may not store EUI)
	KEY128 nwkSKey;			///< shared session key 16 bytes
	KEY128 appSKey;			///< private key 16 bytes
	LORAWAN_VERSION version;
	// OTAA
	DEVEUI appEUI;			///< OTAA application identifier
	KEY128 appKey;			///< OTAA application private key
    KEY128 nwkKey;          ///< OTAA network key
	DEVNONCE devNonce;      ///< last device nonce
	JOINNONCE joinNonce;    ///< last Join nonce
	// added for searching
	DEVICENAME name;

	size_t operator()(const DEVICEID &value) const {
		return value.devEUI.u;
	}
	bool operator==(const DEVICEID &rhs) const {
		return rhs.devEUI.u == devEUI.u;
	}
	bool operator==(const DEVEUI &rhs) const {
		return rhs == devEUI;
	}
	bool operator<(const DEVICEID &rhs) const {
		return rhs.devEUI.u < devEUI.u;
	}
	bool operator>(const DEVICEID &rhs) const {
		return rhs.devEUI.u > devEUI.u;
	}
	bool operator!=(const DEVICEID &rhs) const {
		return rhs.devEUI.u != devEUI.u;
	}

	DEVICEID();
    DEVICEID(
        ACTIVATION activation,
        DEVICECLASS deviceclass,
        const DEVEUI &devEUI,
        const KEY128 &nwkSKey,
        const KEY128 &appSKey,
        LORAWAN_VERSION version,
        const DEVEUI &appEUI,
        const KEY128 &appKey,
        const KEY128 &nwkKey,
        DEVNONCE devNonce,
        const JOINNONCE joinNonce,
        const DEVICENAME name
    );
    // ABP
    DEVICEID(
        DEVICECLASS deviceclass,
        const DEVEUI &devEUI,
        const KEY128 &nwkSKey,
        const KEY128 &appSKey,
        LORAWAN_VERSION version,
        const DEVICENAME name
    );

	DEVICEID(const DEVICEID &value);
	DEVICEID& operator=(const DEVICEID& other);
	DEVICEID& operator=(const NETWORKIDENTITY& other);

	void set(const DEVICEID &value);
	void setEUIString(const std::string &value);
	void setNwkSKeyString(const std::string &value);
	void setAppSKeyString(const std::string &value);
	void setName(const std::string &value);
	void setClass(const DEVICECLASS &value);

	std::string toString() const;
    std::string toString(const DEVADDR &addr) const;
	std::string toJsonString() const;
    std::string toJsonString(const DEVADDR &addr) const;
	void setProperties(std::map<std::string, std::string> &retval) const;

    bool empty() const;
});					// 44 bytes + 8 + 18 = 70

#define SIZE_DEVICEID 70

PACK(class NETWORKIDENTITY {
public:
	// key
	DEVADDR devaddr;		///< network address 4 bytes
	DEVICEID devid;         // 91 bytes

	NETWORKIDENTITY();
	NETWORKIDENTITY(const DEVADDR &a, const DEVICEID &id);
    NETWORKIDENTITY(const NETWORKIDENTITY &id);
    explicit NETWORKIDENTITY(const DEVICEID &id);
    explicit NETWORKIDENTITY(const DEVADDR &addr);
	void set(const NETWORKIDENTITY &id);
	void set(const DEVADDR &addr, const DEVICEID &value);
	std::string toString() const;
    std::string toJsonString() const;
});  // 95 bytes

#define SIZE_NETWORKIDENTITY 96

typedef PACK(struct {
     uint16_t vendorId;
     uint16_t vendorProfileId;
} ) PROFILE_TYPE;		// 4 bytes

PACK(class PROFILEID {
     public:
         union {
             unsigned char c[4];
             uint32_t u;
             PROFILE_TYPE profile;
         };
         PROFILEID();
         explicit PROFILEID(const std::string &hex);
         explicit PROFILEID(uint32_t value);
         std::size_t operator()(const PROFILEID &value) const;
         bool operator==(const PROFILEID &rhs) const;
         bool operator<(const PROFILEID &rhs) const;
         bool operator>(const PROFILEID &rhs) const;
         bool operator!=(const PROFILEID &rhs) const;
});

#define SIZE_CONFIRMATION_EMPTY_DOWN 16

#endif
