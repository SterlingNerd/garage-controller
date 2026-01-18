idf_component_register(
    SRCS "esp_zb_light.c"
    INCLUDE_DIRS "." "../light"
    PRIV_REQUIRES esp-zigbee-lib esp-zboss-lib zcl_utility light nvs_flash
)