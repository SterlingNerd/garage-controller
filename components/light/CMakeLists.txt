idf_component_register(
    SRCS "light_driver.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES driver espressif__led_strip
)