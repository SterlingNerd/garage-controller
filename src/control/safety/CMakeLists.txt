idf_component_register(
    SRCS "safety_monitor.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_timer
)