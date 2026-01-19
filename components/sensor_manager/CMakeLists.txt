idf_component_register(
    SRCS "sensor_manager.c" "sensor_interface.c"
    INCLUDE_DIRS "." "../devices/dht22"
    PRIV_REQUIRES esp_timer dht22
)