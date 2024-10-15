#ifndef LORAWAN_CONST_H_
#define LORAWAN_CONST_H_	1

#define FPORT_NO_PAYLOAD   0

typedef enum MODULATION {
    MODULATION_UNDEFINED = 0,
    MODULATION_LORA = 0x10,
    MODULATION_FSK = 0x20
} MODULATION;

/**
 * not sure for BANDWIDTH_INDEX_7KHZ..BANDWIDTH_INDEX_125KHZ
 * @see https://github.com/x893/SX1231/blob/master/SX12xxDrivers-2.0.0/src/radio/sx1276-LoRa.c
 * SignalBw
 * 0: 7.8kHz, 1: 10.4 kHz, 2: 15.6 kHz, 3: 20.8 kHz, 4: 31.2 kHz,
 * 5: 41.6 kHz, 6: 62.5 kHz, 7: 125 kHz, 8: 250 kHz, 9: 500 kHz, other: Reserved
 */ 
typedef enum BANDWIDTH {
    BANDWIDTH_INDEX_7KHZ   = 0,   // 7.8
    BANDWIDTH_INDEX_10KHZ  = 1,   // 10.4
    BANDWIDTH_INDEX_15KHZ  = 2,   // 15.6
    BANDWIDTH_INDEX_20KHZ  = 3,   // 20.8
    BANDWIDTH_INDEX_31KHZ  = 4,   // 31.2
    BANDWIDTH_INDEX_41KHZ  = 5,   // 41.6
    BANDWIDTH_INDEX_62KHZ  = 6,   // 62.5
    BANDWIDTH_INDEX_125KHZ = 7,   // 125
    BANDWIDTH_INDEX_250KHZ = 8,   // 250
    BANDWIDTH_INDEX_500KHZ = 9    // 500
} BANDWIDTH;

typedef enum SPREADING_FACTOR {
    DRLORA_SF5 = 5,
    DRLORA_SF6 = 6,
    DRLORA_SF7 = 7,
    DRLORA_SF8 = 8,
    DRLORA_SF9 = 9,
    DRLORA_SF10 = 10,
    DRLORA_SF11 = 11,
    DRLORA_SF12 = 12
} SPREADING_FACTOR;

typedef enum CODING_RATE {
    CRLORA_0FF    = 0,
    CRLORA_4_5    = 1,
    CRLORA_4_6    = 2,   // default?
    CRLORA_4_7    = 3,
    CRLORA_4_8    = 4,
    CRLORA_LI_4_5 = 5,
    CRLORA_LI_4_6 = 6,
    CRLORA_LI_4_8 = 7
} CODING_RATE;

#define DATA_RATE_SIZE              8

#define TX_POWER_OFFSET_MAX_SIZE    16

typedef enum GW_STATUS {
    TXSTATUS_UNKNOWN,
    TXOFF,
    TXFREE,
    TXSCHEDULED,
    TXEMITTING,
    RXSTATUS_UNKNOWN,
    RXOFF,
    RXON,
    RXSUSPENDED
} GW_STATUS;

typedef enum CRC_STATUS {
    CRC_STATUS_UNDEFINED  = 0,
    CRC_STATUS_NO_CRC     = 1,
    CRC_STATUS_CRC_BAD    = 0x11,
    CRC_STATUS_CRC_OK     = 0x10
} CRC_STATUS;

typedef enum TEMPERATURE_SRC {
    TEMP_SRC_EXT,   // the temperature has been measured with an external sensor
    TEMP_SRC_MCU    // the temperature has been measured by the gateway MCU
} TEMPERATURE_SRC;

#endif
