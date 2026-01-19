idf_component_register(
    SRCS "zigbee_manager.c"
    INCLUDE_DIRS "." "../devices/light" "../sensor_manager"
    PRIV_REQUIRES esp-zigbee-lib esp-zboss-lib zcl_utility light nvs_flash sensor_manager
)