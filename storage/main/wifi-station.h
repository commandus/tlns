
#include "esp_wifi.h"

/**
 * @brief Establish Wi-Fi network connection (station mode)
 * 
 * @return IPv4 address esp_ip4_addr_t !=0 if success
 */
extern "C" esp_ip4_addr_t wifiStationInit();
