/*
 * DHT22 Temperature and Humidity Sensor Driver Implementation
 *
 * Implements the DHT22 single-wire communication protocol
 */

#include "dht22.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "DHT22";

#define DHT22_TIMEOUT_US 1000000  // 1 second timeout
#define DHT22_DATA_BITS 40        // 5 bytes * 8 bits

// Timing constants for DHT22 protocol (microseconds)
#define DHT22_START_SIGNAL_LOW_US 1100
#define DHT22_START_SIGNAL_HIGH_US 30
#define DHT22_RESPONSE_LOW_US 80
#define DHT22_RESPONSE_HIGH_US 80
#define DHT22_BIT_0_HIGH_US 26
#define DHT22_BIT_1_HIGH_US 70

static int dht22_gpio_pin = -1;
static bool dht22_initialized = false;

static inline void delay_us(uint32_t us) {
    uint64_t start = esp_timer_get_time();
    while ((esp_timer_get_time() - start) < us) {
        // Busy wait
    }
}

static bool dht22_wait_for_level(gpio_num_t pin, int level, uint32_t timeout_us) {
    uint64_t start = esp_timer_get_time();
    while (gpio_get_level(pin) != level) {
        if ((esp_timer_get_time() - start) > timeout_us) {
            return false;
        }
    }
    return true;
}

static int dht22_read_bit(gpio_num_t pin) {
    // Wait for the pin to go low (start of bit)
    if (!dht22_wait_for_level(pin, 0, 100)) {
        return -1;
    }

    // Measure how long it stays low
    uint64_t start = esp_timer_get_time();
    if (!dht22_wait_for_level(pin, 1, 100)) {
        return -1;
    }
    uint32_t low_time = esp_timer_get_time() - start;

    // Wait for the pin to go low again (end of bit)
    if (!dht22_wait_for_level(pin, 0, 100)) {
        return -1;
    }

    // DHT22 protocol: bit 0 has ~26us high time, bit 1 has ~70us high time
    return (low_time > 40) ? 1 : 0;
}

bool dht22_init(int gpio_pin) {
    if (gpio_pin < 0 || gpio_pin > 47) {
        ESP_LOGE(TAG, "Invalid GPIO pin: %d", gpio_pin);
        return false;
    }

    dht22_gpio_pin = gpio_pin;

    // Configure GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_pin),
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,  // Open drain for DHT22 protocol
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO %d: %s", gpio_pin, esp_err_to_name(ret));
        return false;
    }

    // Set pin high initially (idle state)
    gpio_set_level(gpio_pin, 1);

    dht22_initialized = true;
    ESP_LOGI(TAG, "DHT22 initialized on GPIO %d", gpio_pin);
    return true;
}

bool dht22_read(dht22_reading_t *reading) {
    if (!dht22_initialized || !reading) {
        return false;
    }

    gpio_num_t pin = (gpio_num_t)dht22_gpio_pin;

    // Send start signal
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);  // Pull low
    delay_us(DHT22_START_SIGNAL_LOW_US);

    gpio_set_level(pin, 1);  // Release (pull-up will make it high)
    delay_us(DHT22_START_SIGNAL_HIGH_US);

    // Switch to input mode
    gpio_set_direction(pin, GPIO_MODE_INPUT);

    // Wait for DHT22 response
    if (!dht22_wait_for_level(pin, 0, 100)) {
        ESP_LOGW(TAG, "Timeout waiting for DHT22 response (low)");
        return false;
    }

    if (!dht22_wait_for_level(pin, 1, 100)) {
        ESP_LOGW(TAG, "Timeout waiting for DHT22 response (high)");
        return false;
    }

    // Read 40 bits of data
    uint8_t data[5] = {0};
    for (int byte = 0; byte < 5; byte++) {
        for (int bit = 0; bit < 8; bit++) {
            int bit_value = dht22_read_bit(pin);
            if (bit_value < 0) {
                ESP_LOGW(TAG, "Failed to read bit %d of byte %d", bit, byte);
                return false;
            }
            data[byte] |= (bit_value << (7 - bit));
        }
    }

    // Verify checksum
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        ESP_LOGW(TAG, "DHT22 checksum failed: calculated %d, received %d", checksum, data[4]);
        return false;
    }

    // Convert raw data to temperature and humidity
    uint16_t raw_humidity = (data[0] << 8) | data[1];
    uint16_t raw_temperature = (data[2] << 8) | data[3];

    reading->humidity_percent = raw_humidity / 10.0f;

    // Handle negative temperatures (MSB of temperature indicates sign)
    if (raw_temperature & 0x8000) {
        reading->temperature_c = -(raw_temperature & 0x7FFF) / 10.0f;
    } else {
        reading->temperature_c = raw_temperature / 10.0f;
    }

    reading->valid = true;

    ESP_LOGD(TAG, "DHT22 reading: Temperature=%.1f°C, Humidity=%.1f%%",
             reading->temperature_c, reading->humidity_percent);

    return true;
}

bool dht22_is_available(void) {
    if (!dht22_initialized) {
        return false;
    }

    // Try a quick read to test if sensor is responding
    dht22_reading_t test_reading;
    return dht22_read(&test_reading);
}