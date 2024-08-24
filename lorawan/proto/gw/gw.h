#ifndef GW_H
#define GW_H

#include "lorawan/lorawan-packet-storage.h"

/**
 * Gateway push data is message upstream from the device to the server
 */
class GwPushData {
public:
    LORAWAN_MESSAGE_STORAGE rxData;             ///< received message itself
    SEMTECH_PROTOCOL_METADATA_RX rxMetadata;    ///< received message metadata including receiving time, frequency, RSSI etc.
    bool needConfirmation() const;
    bool isMetadataValid() const;
};

/**
 * Gateway message downstream from the server to the device
 */
class GwPullResp {
public:
    DEVEUI gwId;                                ///< gateway selected for transmission (closest to the device or having the best conditions for transmission)
    LORAWAN_MESSAGE_STORAGE txData;             ///< message to be send
    SEMTECH_PROTOCOL_METADATA_TX txMetadata;    ///< transmission frequency etc
    bool needConfirmation() const;
    bool isMetadataValid() const;
};

#endif
