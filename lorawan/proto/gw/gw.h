//
// Created by andrei on 01.02.24.
//

#ifndef GW_H
#define GW_H

#include "lorawan/lorawan-packet-storage.h"

class GwPushData {
public:
    LorawanPacketStorage rxData;
    SEMTECH_PROTOCOL_METADATA_RX rxMetadata;
};

class GwPullResp {
public:
    DEVEUI gwId;
    LorawanPacketStorage txData;
    SEMTECH_PROTOCOL_METADATA_TX txMetadata;
};

#endif //TLNS_GW_H
