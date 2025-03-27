#ifndef TLNS_GATEWAY_SETTINGS_HELPER_H
#define TLNS_GATEWAY_SETTINGS_HELPER_H

#include <string>
#include "gateway-settings.h"

size_t findGatewayRegionIndex(
    const GatewaySettings *lorawanGatewaySettings,
    const std::string &namePrefix
);

#endif
