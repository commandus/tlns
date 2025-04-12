#include "gateway-settings-helper.h"
#include <algorithm>

size_t findGatewayRegionIndex(
    const GatewaySettings *lorawanGatewaySettings,
    const std::string &namePrefix
)
{
    std::string upperPrefix(namePrefix);
    std::transform(upperPrefix.begin(), upperPrefix.end(), upperPrefix.begin(), ::toupper);
    for (size_t i = 0; i < sizeof(*lorawanGatewaySettings) / sizeof(GatewaySettings*); i++) {
        std::string upperName(lorawanGatewaySettings[i].name);
        std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
        if (upperName.find(upperPrefix) != std::string::npos)
            return i;
    }
    return 0;
}
