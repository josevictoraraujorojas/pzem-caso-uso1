idf_component_register(
    SRCS "pzem-driver.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi nvs_flash esp_netif mqtt driver
)