#ifndef TLNS_USB_LISTENER_H
#define TLNS_USB_LISTENER_H

#include "gateway-settings.h"
#include "gw-dev/usb/usb-lora-gw.h"

enum UsbListenerState {
    USB_LISTENER_STATE_STOPPED = 0,
    USB_LISTENER_STATE_RUNNING,
    USB_LISTENER_STATE_STOP_REQUEST
};

class UsbListener : public UsbLoRaWANGateway {
private:
    std::thread *upstreamThread;
    int runner();
public:
    UsbListenerState state;

    /**
     * Default constructor, call init() to set regional parameters.
     */
    UsbListener();
    UsbListener(const UsbListener&);
    virtual ~UsbListener();
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

#endif
