#include <thread>
#include "lorawan/downlink/downlink-by-timer.h"
#include "lorawan/helper/passphrase.h"

DownlinkByTimer::DownlinkByTimer(
    MessageTaskDispatcher *dispatcher,
    DirectClient *aIdentityClient,
    uint32_t aSeconds
)
    : RunDownlink(dispatcher), identityClient(aIdentityClient), seconds(aSeconds), index(0)
{
    identityClient->svcIdentity->list(identities, 0, 100);
}

void DownlinkByTimer::run()
{
    while (state != DLRS_STOP) {
        if (index >= identities.size())
            index = 0;
        else {
            TASK_TIME timeNow = std::chrono::system_clock::now();
            DEVADDR devAddr = identities[index].value.devaddr;

            std::string s = getPassphrase();

            void *payload = (void *) s.c_str();
            void *fOpts = nullptr;
            uint8_t fPort = 2;
            uint8_t payloadSize = (uint8_t) s.size();
            uint8_t fOptsSize = 0;
            dispatcher->sendDownlink(timeNow, devAddr, payload, fOpts, fPort, payloadSize, fOptsSize);
            index++;
        }
#ifdef _MSC_VER
        Sleep(seconds * 1000);
#else
        sleep(seconds);
#endif
    }
}
