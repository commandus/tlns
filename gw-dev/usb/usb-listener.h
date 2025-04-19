#ifndef TLNS_USB_LISTENER_H
#define TLNS_USB_LISTENER_H

#include <mutex>
#include <condition_variable>

#include "gateway-settings.h"

enum UsbListenerState {
    USB_LISTENER_STATE_STOPPED = 0,
    USB_LISTENER_STATE_RUNNING,
    USB_LISTENER_STATE_STOP_REQUEST
};

class UsbListener {
private:
    std::mutex mutexState;
    std::condition_variable cvState;
    GatewaySettings *gatewaySettings;
    UsbListenerState state;
    int runner();
public:
    uint64_t eui;
    /**
     * Default constructor, call init() to set regional parameters.
     */
    UsbListener();
    UsbListener(const UsbListener&);
    ~UsbListener();
    /**
     * Initialize or re-initialize listener. If listener is running, it stops.
     * @param gatewaySettings regional settings
     * @return 0- success
     */
    int init(GatewaySettings *gatewaySettings);
    /**
     * Start listener thread. Listener must be initialized first.
     * @return 0- success. <0- error code.
     */
    int start();
    /**
     * Stop listener thread
     * @param aWait true- wait thread has been stopped
     * @return 0 -success
     */
    int stop(bool aWait);

    void wait();
};

#endif //TLNS_USB_LISTENER_H
