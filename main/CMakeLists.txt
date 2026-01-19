idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "." "../components/zigbee" "../components/devices/light" "../components/devices/dht22" "../components/sensor_manager"
    PRIV_REQUIRES nvs_flash esp_driver_uart ieee802154 esp-zigbee-lib esp-zboss-lib zcl_utility zigbee light dht22 sensor_manager
)
