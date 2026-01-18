idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "." "../components/zigbee" "../components/light"
    PRIV_REQUIRES nvs_flash esp_driver_uart ieee802154 esp-zigbee-lib esp-zboss-lib zcl_utility zigbee light
)
