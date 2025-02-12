#ifndef IDENTITY_SERVICE_C_WRAPPER_H
#define IDENTITY_SERVICE_C_WRAPPER_H       1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <inttypes.h>

#if defined(_MSC_VER) || defined(__MINGW32__)
#ifdef __cplusplus
#define EXPORT_SHARED_C_FUNC extern "C" __declspec(dllexport)
#else
#define EXPORT_SHARED_C_FUNC __declspec(dllexport)
#endif
#else
#ifdef __cplusplus
#define EXPORT_SHARED_C_FUNC extern "C"
#else
#define EXPORT_SHARED_C_FUNC
#endif
#endif

// Structures

#ifdef __GNUC__
#define PACK_STRUCT( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#else
#ifdef _MSC_VER
#define PACK_STRUCT( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#else
#define PACK_STRUCT( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif
#endif

typedef uint64_t C_DEVEUI;
typedef uint32_t C_DEVADDR;
typedef uint8_t C_ACTIVATION;
typedef uint8_t C_DEVICECLASS;
typedef uint8_t C_LORAWAN_VERSION;
typedef uint8_t C_KEY128[16];
typedef uint16_t C_DEVNONCE;
typedef uint8_t C_JOINNONCE[3];
typedef uint8_t C_NETID[3];
typedef char C_DEVICENAME[8];

typedef PACK_STRUCT( struct {
    C_ACTIVATION activation;	///< activation type: ABP or OTAA
    C_DEVICECLASS deviceclass;
    C_DEVEUI devEUI;		    ///< device identifier 8 bytes (ABP device may not store EUI)
    C_KEY128 nwkSKey;			///< shared session key 16 bytes
    C_KEY128 appSKey;			///< private key 16 bytes
    C_LORAWAN_VERSION version;
    // OTAA
    C_DEVEUI appEUI;			///< OTAA application identifier
    C_KEY128 appKey;			///< OTAA application private key
    C_KEY128 nwkKey;            ///< OTAA network key
    C_DEVNONCE devNonce;        ///< last device nonce
    C_JOINNONCE joinNonce;      ///< last Join nonce
    C_DEVICENAME name;
} ) C_DEVICEID;

typedef PACK_STRUCT( struct {
    C_DEVADDR devaddr;		///< network address 4 bytes
    C_DEVICEID devid;         // 91 bytes
} ) C_NETWORKIDENTITY;

typedef enum C_NETWORK_IDENTITY_COMPARISON_OPERATOR {
    C_NICO_NONE = 0,
    C_NICO_EQ,
    C_NICO_NE,
    C_NICO_GT,
    C_NICO_LT,
    C_NICO_GE,
    C_NICO_LE
} C_NETWORK_IDENTITY_COMPARISON_OPERATOR;

typedef enum C_NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR {
    C_NILPO_NONE = 0,
    C_NILPO_AND,
    C_NILPO_OR
} C_NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR;

typedef enum C_NETWORK_IDENTITY_PROPERTY {
    C_NIP_NONE = 0,
    C_NIP_ADDRESS = 1,
    C_NIP_ACTIVATION = 2,     ///< activation type: ABP or OTAA
    C_NIP_DEVICE_CLASS = 3,   ///< A, B, C
    C_NIP_DEVEUI = 4,		    ///< device identifier 8 bytes (ABP device may not store EUI)
    C_NIP_NWKSKEY = 5,		///< shared session key 16 bytes
    C_NIP_APPSKEY = 6,        ///< private key 16 bytes
    C_NIP_LORAWAN_VERSION = 7,
    // OTAA
    C_NIP_APPEUI = 8,			///< OTAA application identifier
    C_NIP_APPKEY = 9,			///< OTAA application private key
    C_NIP_NWKKEY = 10,        ///< OTAA network key
    C_NIP_DEVNONCE = 11,      ///< last device nonce
    C_NIP_JOINNONCE = 12,     ///< last Join nonce
    // added for searching
    C_NIP_DEVICENAME = 13
} C_NETWORK_IDENTITY_PROPERTY;

typedef PACK_STRUCT( struct {
    union
    {
        uint8_t c;
        struct
        {
            uint8_t RX2DataRate: 4;	    ///< downlink data rate that serves to communicate with the end-device on the second receive window (RX2)
            uint8_t RX1DROffset: 3;	    ///< offset between the uplink data rate and the downlink data rate used to communicate with the end-device on the first receive window (RX1)
            uint8_t optNeg: 1;     	    ///< 1.0- RFU, 1.1- optNeg
        };
    };
} ) C_DLSETTINGS;	        // 1 byte

typedef PACK_STRUCT( struct {
    C_JOINNONCE joinNonce;   	        //
    C_NETID netId;   	                //
    C_DEVADDR devAddr;   	            //
    C_DLSETTINGS dlSettings;		    // downlink configuration settings
    uint8_t rxDelay;                //
} ) C_JOIN_ACCEPT_FRAME_HEADER;	    // 3 3 4 1 1 = 12 bytes

typedef PACK_STRUCT( struct {
    enum C_NETWORK_IDENTITY_LOGICAL_PRE_OPERATOR pre;  ///< and/or previous statement
    enum C_NETWORK_IDENTITY_PROPERTY property;
    enum C_NETWORK_IDENTITY_COMPARISON_OPERATOR comparisonOperator;
    uint8_t length; // 0..16
    char filterData[16];
} ) C_NETWORK_IDENTITY_FILTER;    // 20 bytes long

// Functions

typedef enum C_IDENTITY_SERVICE_IMPL {
    CISI_GEN = 0,
    CISI_JSON = 1,
    CISI_MEM = 2,
    CISI_SQLITE = 3,
    CISI_UDP = 4,
    CISI_LMDB = 5
} C_IDENTITY_SERVICE_IMPL;

EXPORT_SHARED_C_FUNC void* makeIdentityServiceC(
    C_IDENTITY_SERVICE_IMPL impl
);

EXPORT_SHARED_C_FUNC void destroyIdentityServiceC(
    void *instance
);

EXPORT_SHARED_C_FUNC int c_get(void *o, C_DEVICEID *retVal, const C_DEVADDR *devAddr);
EXPORT_SHARED_C_FUNC int c_getNetworkIdentity(void *o, C_NETWORKIDENTITY *retVal, const C_DEVEUI *eui);
EXPORT_SHARED_C_FUNC int c_put(void *o, const C_DEVADDR *devaddr, const C_DEVICEID *id);
EXPORT_SHARED_C_FUNC int c_rm(void *o, const C_DEVADDR *addr);
EXPORT_SHARED_C_FUNC int c_list(void *o, C_NETWORKIDENTITY retVal[], uint32_t offset, uint8_t size);
EXPORT_SHARED_C_FUNC int c_filter(
    void *o,
    C_NETWORKIDENTITY retVal[],
    C_NETWORK_IDENTITY_FILTER filters[],
    size_t filterSize,
    uint32_t offset,
    uint8_t size
);

EXPORT_SHARED_C_FUNC int c_filterExpression(
    void *o,
    C_NETWORKIDENTITY retVal[],
    const char *filterExpression,
    size_t filterExpressionSize,
    uint32_t offset,
    uint8_t size
);

EXPORT_SHARED_C_FUNC size_t c_size(void *o);
EXPORT_SHARED_C_FUNC int c_next(void *o, C_NETWORKIDENTITY *retVal);
EXPORT_SHARED_C_FUNC void c_flush(void *o);
EXPORT_SHARED_C_FUNC int c_init(void *o, const char *option, void *data);
EXPORT_SHARED_C_FUNC void c_done(void *o);
EXPORT_SHARED_C_FUNC void c_setOption(void *o, int option, void *value);
EXPORT_SHARED_C_FUNC C_NETID *c_getNetworkId(void *o);
EXPORT_SHARED_C_FUNC void c_setNetworkId(void *o, const C_NETID *value);
EXPORT_SHARED_C_FUNC int c_joinAccept(void *o, C_JOIN_ACCEPT_FRAME_HEADER *retVal, C_NETWORKIDENTITY *networkIdentity);

EXPORT_SHARED_C_FUNC void text2c_devaddr(
    C_DEVADDR *retVal,
    const char *expr
);

EXPORT_SHARED_C_FUNC void text2c_deviceid(
    C_DEVICEID *retVal,
    const char *lines[]
);

EXPORT_SHARED_C_FUNC void text2c_networkidentity(
    C_NETWORKIDENTITY *retVal,
    const char *lines[]
);

EXPORT_SHARED_C_FUNC size_t c_devaddr2text(
    char *buffer,
    size_t bufferSize,
    C_DEVADDR *addr
);

EXPORT_SHARED_C_FUNC void c_deviceid2text(
    char *buffer,
    size_t bufferSize,
    char *retVal[],
    C_DEVICEID *value
);

EXPORT_SHARED_C_FUNC void c_networkidentity2text(
    char *buffer,
    size_t bufferSize,
    char *retVal[],
    C_NETWORKIDENTITY *value
);

#ifdef __cplusplus
}
#endif

#endif
