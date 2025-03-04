#include <thread>
#include "lorawan/downlink/downlink-by-timer.h"

#ifdef _MSC_VER
#define sleep(x) Sleep(x)
#endif

DownlinkByTimer::DownlinkByTimer(
    MessageTaskDispatcher *dispatcher,
    DirectClient *aIdentityClient,
    uint32_t aSeconds
)
    : RunDownlink(dispatcher), identityClient(aIdentityClient), seconds(seconds), index(0)
{
    identityClient->svcIdentity->list(identities, 0, 100);
}

void DownlinkByTimer::run()
{
    while (state != DLRS_STOP) {
        if (index >= identities.size())
            index == 0;
        else {
            identities[index];
            // dispatcher->sendDownlink()
            index++;
        }
        sleep(seconds);
    }
}
