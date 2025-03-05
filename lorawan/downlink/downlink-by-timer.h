#ifndef TLNS_DOWNLINK_BY_TIMER_H
#define TLNS_DOWNLINK_BY_TIMER_H

#include "lorawan/downlink/run-downlink.h"
#include "lorawan/storage/client/direct-client.h"

class DownlinkByTimer : public RunDownlink {
private:
    DirectClient *identityClient;
    uint32_t seconds;
    std::vector<NETWORKIDENTITY> identities;
    size_t index;
public:
    DownlinkByTimer(
        MessageTaskDispatcher *dispatcher,
        DirectClient *identityClient,
        uint32_t seconds = 1
    );
    void run() override;
};

#endif //TLNS_DOWNLINK_BY_TIMER_H
