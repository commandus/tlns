#ifndef TLNS_USB_LORA_GATEWAY_H
#define TLNS_USB_LORA_GATEWAY_H

#include "gateway-settings.h"

class UsbLoRaWANGateway {
public:
    GatewaySettings *gatewaySettings;
    uint64_t eui;
    /**
     * Default constructor, call init() to set regional parameters.
     */
    UsbLoRaWANGateway();
    UsbLoRaWANGateway(GatewaySettings *gatewaySettings, uint64_t eui);
    UsbLoRaWANGateway(const UsbLoRaWANGateway&);
    virtual ~UsbLoRaWANGateway();
    /**
     * Initialize or re-initialize listener. If listener is running, it stops.
     * @param gatewaySettings regional settings
     * @return 0- success
     */
    int init(GatewaySettings *gatewaySettings);
};

#endif
