#ifndef TLNS_USB_LISTENER_H
#define TLNS_USB_LISTENER_H

#include <mutex>
#include "gateway-settings.h"

enum UsbListenerState {
    USB_LISTENER_STATE_STOPPED = 0,
    USB_LISTENER_STATE_RUNNING,
    USB_LISTENER_STATE_STOP_REQUEST
};

class UsbListener {
private:
    std::mutex mutexState;
    std::mutex mLGW;
    std::condition_variable cvState;
    GatewaySettings *gatewaySettings;
    UsbListenerState state;
    int runner();
public:
    UsbListener();
    virtual ~UsbListener();
    int init(GatewaySettings *gatewaySettings);
    int start();
    int stop();
};

#endif //TLNS_USB_LISTENER_H
