#include "sdkconfig.h"

#ifndef CONFIG_ESP_WIFI_SSID
#define CONFIG_ESP_WIFI_SSID            "ikfia-guest"
#endif

#ifndef CONFIG_ESP_WIFI_PASS
#define CONFIG_ESP_WIFI_PASS            "f95nder9"
#endif

#ifndef CONFIG_ESP_SOCK_TARGET_HOST
#define CONFIG_ESP_SOCK_TARGET_HOST     "84.237.104.128"
#endif

#ifndef CONFIG_ESP_UDP_SOCK_PORT
#define CONFIG_ESP_UDP_SOCK_PORT        4242
#endif

#ifndef CONFIG_ESP_MAXIMUM_RETRY
#define CONFIG_ESP_MAXIMUM_RETRY        5
#endif

#ifndef CONFIG_LOG_MAXIMUM_LEVEL
#define CONFIG_LOG_MAXIMUM_LEVEL        3
#endif

#ifndef CONFIG_ESP_WPA3_SAE_PWE_BOTH
#define CONFIG_ESP_WPA3_SAE_PWE_BOTH    y
#endif

#ifndef CONFIG_ESP_WIFI_PW_ID
#define CONFIG_ESP_WIFI_PW_ID           ""
#endif

#ifndef CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define CONFIG_ESP_WIFI_AUTH_WPA2_PSK   y
#endif

#ifndef CONFIG_ESP_KEY_GEN
#define CONFIG_ESP_KEY_GEN     y
#endif

#ifndef CONFIG_ESP_PASSPHRASE
#define CONFIG_ESP_PASSPHRASE     "masterkey"
#endif

#ifndef CONFIG_ESP_NWK_TYPE_ID
#define CONFIG_ESP_NWK_TYPE_ID     0
#endif

#ifndef CONFIG_ESP_NWK_ID
#define CONFIG_ESP_NWK_ID     0
#endif

#ifndef CONFIG_ESP_CODE
#define CONFIG_ESP_CODE     42
#endif

#ifndef CONFIG_ESP_ACCESS_CODE_HI
#define CONFIG_ESP_ACCESS_CODE_HI     0
#endif

#ifndef CONFIG_ESP_ACCESS_CODE_LO
#define CONFIG_ESP_ACCESS_CODE_LO     42
#endif

#define CONFIG_ESP_ACCESS_CODE (((uint64_t) CONFIG_ESP_ACCESS_CODE_HI << 32) | (uint64_t) CONFIG_ESP_ACCESS_CODE_LO)

#ifndef CONFIG_ESP_ENABLE_DEBUG
#define ENABLE_DEBUG     n
#endif
#define ENABLE_DEBUG     n

