/*
 * DHT22 Temperature and Humidity Sensor Driver
 *
 * Driver for DHT22 sensor providing temperature and humidity readings
 */

#ifndef DHT22_H
#define DHT22_H

#include <stdbool.h>

// DHT22 sensor data structure
typedef struct {
    float temperature_c;
    float humidity_percent;
    bool valid;
} dht22_reading_t;

// DHT22 driver functions
bool dht22_init(int gpio_pin);
bool dht22_read(dht22_reading_t *reading);
bool dht22_is_available(void);

#endif // DHT22_H