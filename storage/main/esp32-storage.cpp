#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"

#include "platform-defs.h"
#include "wifi-station.h"
#include "lorawan/lorawan-error.h"

#ifdef CONFIG_ESP_KEY_GEN
#include "lorawan/storage/service/identity-service-gen.h"
#else
#include "identity-service-mem.h"
#endif
#include "lorawan/storage/listener/udp-listener.h"
#include "lorawan/storage/serialization/gateway-serialization.h"
#include "lorawan/storage/service/gateway-service-mem.h"

#define TAG "lorawan-storage"

/**
 * Run identity(with GEN or in memory) & gateway storage
 */
extern "C" int app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    esp_ip4_addr_t ip = wifiStationInit();
    if (ip.addr == 0)
      ret = ERR_CODE_ADDR_OUT_OF_RANGE;
    ESP_ERROR_CHECK(ret);
    auto identityService =
#ifdef CONFIG_ESP_KEY_GEN
        new GenIdentityService;
#else
        new MemoryIdentityService;
#endif
#ifdef CONFIG_ESP_KEY_GEN
    NETID netid(CONFIG_ESP_NWK_TYPE_ID, CONFIG_ESP_NWK_ID);
    identityService->init(CONFIG_ESP_PASSPHRASE, &netid);
#else
    identityService->init("", nullptr);
#endif

    auto gatewayService = new MemoryGatewayService;
    gatewayService->init("", nullptr);

    IdentitySerialization identitySerializatiom(identityService, CONFIG_ESP_CODE, CONFIG_ESP_ACCESS_CODE);
    GatewaySerialization gatewaySerializatiom(gatewayService, CONFIG_ESP_CODE, CONFIG_ESP_ACCESS_CODE);

    ESP_LOGI(TAG, IPSTR ":%u master key: %s, net: %s, code: %u, access code: %llu", 
      IP2STR(&ip), CONFIG_ESP_UDP_SOCK_PORT, CONFIG_ESP_PASSPHRASE, netid.toString().c_str(), CONFIG_ESP_CODE, CONFIG_ESP_ACCESS_CODE);

    UDPListener lsnr(&identitySerializatiom, &gatewaySerializatiom);
    lsnr.setAddress(ip.addr, CONFIG_ESP_UDP_SOCK_PORT);
    lsnr.run();
    return 0;
}
