idf_component_register(
    SRCS "light_driver.c" "light_manager.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES driver espressif__led_strip esp-zigbee-lib zcl_utility
)