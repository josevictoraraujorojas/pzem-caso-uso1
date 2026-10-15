idf_component_register(
    SRCS "hello_world_main.c"
    INCLUDE_DIRS "."
    REQUIRES
        pzem-drivers
        nvs_flash
        esp_netif
        esp_event
        esp_wifi
        mqtt
)
