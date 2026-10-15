idf_component_register(
    SRCS "pzem-driver.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_driver_uart
)