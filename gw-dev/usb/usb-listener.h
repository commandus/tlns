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
    std::condition_variable cvState;
    GatewaySettings *gatewaySettings;
    UsbListenerState state;
    int runner();
public:
    /**
     * Default constructor, call init() to set regional parameters.
     */
    UsbListener();
    /**
     * Initialize listener
     * @param gatewaySettings
     */
    UsbListener(GatewaySettings *gatewaySettings);
    UsbListener(const UsbListener&);
    virtual ~UsbListener();
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
     * @return
     */
    int stop();
};

#endif //TLNS_USB_LISTENER_H
