set(AES_SRC
        ../third-party/system/crypto/aes.c
        ../third-party/system/crypto/cmac.c
)

set(IDENTITY_SRC
        ../lorawan/lorawan-types.cpp
        ../lorawan/lorawan-string.cpp
        ../lorawan/lorawan-conv.cpp
        ../lorawan/storage/network-identity.cpp
        ../lorawan/storage/gateway-identity.cpp
        ../lorawan/storage/service/identity-service.cpp
        ../lorawan/storage/service/gateway-service.cpp
        ../lorawan/storage/service/gateway-service-mem.cpp
        ../lorawan/storage/listener/storage-listener.cpp
        ../lorawan/storage/listener/udp-listener.cpp
        ../lorawan/storage/serialization/identity-serialization.cpp
        ../lorawan/storage/serialization/gateway-serialization.cpp
        ../lorawan/storage/listener/udp-listener.cpp
        ../lorawan/storage/serialization/service-serialization.cpp
        ../lorawan/helper/ip-helper.cpp
        ../lorawan/helper/ip-address.cpp
)

if(CONFIG_ESP_KEY_GEN)
        set(IDENTITY_SRC ${IDENTITY_SRC} ../lorawan/storage/service/identity-service-gen.cpp ../lorawan/helper/key128gen.cpp ${AES_SRC})
else()
        set(IDENTITY_SRC ${IDENTITY_SRC} ../lorawan/storage/service/identity-service-mem.cpp)
endif()

idf_component_register(
SRCS
        esp32-storage.cpp
        wifi-station.c
        ${IDENTITY_SRC}
INCLUDE_DIRS
        "." ".." "../third-party"
REQUIRES
        nvs_flash
        esp_wifi        
)
