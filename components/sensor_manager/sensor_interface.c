/*
 * Sensor Interface Implementation
 *
 * Generic interface implementation for all sensor types
 */

#include "sensor_interface.h"
#include "dht22.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "SENSOR_INTERFACE";

// DHT22 sensor GPIO pin configuration
#define DHT22_GPIO_PIN 0

bool sensor_init(void)
{
    ESP_LOGI(TAG, "Initializing sensor interface");

    // Initialize DHT22 sensor on GPIO0
    if (!dht22_init(DHT22_GPIO_PIN)) {
        ESP_LOGE(TAG, "Failed to initialize DHT22 sensor");
        return false;
    }

    ESP_LOGI(TAG, "Sensor interface initialized successfully");
    return true;
}

bool sensor_read(sensor_type_t type, sensor_reading_t *reading)
{
    if (!reading) {
        return false;
    }

    reading->type = type;
    reading->timestamp = esp_timer_get_time() / 1000; // milliseconds
    reading->valid = false;

    switch (type) {
        case SENSOR_TYPE_ENVIRONMENTAL: {
            dht22_reading_t dht_reading;
            if (dht22_read(&dht_reading)) {
                reading->data.env.temperature_c = dht_reading.temperature_c;
                reading->data.env.humidity_percent = dht_reading.humidity_percent;
                reading->valid = true;
                ESP_LOGD(TAG, "Environmental sensor reading: T=%.1f°C, H=%.1f%%",
                        reading->data.env.temperature_c, reading->data.env.humidity_percent);
                return true;
            } else {
                ESP_LOGW(TAG, "Failed to read DHT22 sensor");
                return false;
            }
        }
        case SENSOR_TYPE_DEPTH:
        case SENSOR_TYPE_VEHICLE_PRESENCE:
        default:
            ESP_LOGW(TAG, "Sensor reading not implemented for type %d", type);
            return false;
    }
}

bool sensor_is_available(sensor_type_t type)
{
    switch (type) {
        case SENSOR_TYPE_ENVIRONMENTAL:
            return dht22_is_available();
        case SENSOR_TYPE_DEPTH:
        case SENSOR_TYPE_VEHICLE_PRESENCE:
        default:
            ESP_LOGW(TAG, "Sensor availability check not implemented for type %d", type);
            return false;
    }
}