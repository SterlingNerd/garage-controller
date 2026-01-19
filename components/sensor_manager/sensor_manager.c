/*
 * Sensor Manager Implementation
 *
 * Handles periodic sensor readings and manages callbacks for sensor updates
 */

#include "sensor_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "SENSOR_MANAGER";

#define MAX_CALLBACKS 4
#define SENSOR_UPDATE_TASK_STACK_SIZE 4096
#define SENSOR_UPDATE_TASK_PRIORITY 5

// Static variables
static sensor_update_callback_t callbacks[MAX_CALLBACKS];
static uint8_t callback_count = 0;
static TaskHandle_t sensor_update_task_handle = NULL;
static uint32_t update_interval_ms = 0;
static sensor_reading_t latest_readings[SENSOR_TYPE_VEHICLE_PRESENCE + 1];

static void sensor_update_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Starting sensor update task");

    // Initial sensor reading after startup delay
    vTaskDelay(pdMS_TO_TICKS(5000));  // Wait 5 seconds after startup

    while (true) {
        // Read environmental sensor
        sensor_reading_t reading;
        if (sensor_read(SENSOR_TYPE_ENVIRONMENTAL, &reading)) {
            // Store latest reading
            latest_readings[SENSOR_TYPE_ENVIRONMENTAL] = reading;

            // Call all registered callbacks
            for (uint8_t i = 0; i < callback_count; i++) {
                if (callbacks[i] != NULL) {
                    callbacks[i](&reading);
                }
            }

            ESP_LOGD(TAG, "Sensor update: T=%.1f°C, H=%.1f%%",
                    reading.data.env.temperature_c, reading.data.env.humidity_percent);
        } else {
            ESP_LOGW(TAG, "Failed to read environmental sensor");
        }

        // Wait for next update interval
        vTaskDelay(pdMS_TO_TICKS(update_interval_ms));
    }
}

bool sensor_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing sensor manager");

    // Initialize the sensor interface (DHT22, etc.)
    if (!sensor_init()) {
        ESP_LOGE(TAG, "Failed to initialize sensor interface");
        return false;
    }

    // Initialize latest readings with invalid data
    for (int i = 0; i <= SENSOR_TYPE_VEHICLE_PRESENCE; i++) {
        latest_readings[i].type = (sensor_type_t)i;
        latest_readings[i].valid = false;
        latest_readings[i].timestamp = 0;
    }

    ESP_LOGI(TAG, "Sensor manager initialized successfully");
    return true;
}

bool sensor_manager_start_updates(uint32_t interval_ms)
{
    if (sensor_update_task_handle != NULL) {
        ESP_LOGW(TAG, "Sensor update task already running");
        return false;
    }

    if (interval_ms == 0) {
        ESP_LOGE(TAG, "Invalid update interval: %lu ms", interval_ms);
        return false;
    }

    update_interval_ms = interval_ms;

    ESP_LOGI(TAG, "Starting sensor updates with %lu ms interval", interval_ms);

    BaseType_t result = xTaskCreate(sensor_update_task,
                                   "sensor_update",
                                   SENSOR_UPDATE_TASK_STACK_SIZE,
                                   NULL,
                                   SENSOR_UPDATE_TASK_PRIORITY,
                                   &sensor_update_task_handle);

    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create sensor update task");
        return false;
    }

    return true;
}

bool sensor_manager_stop_updates(void)
{
    if (sensor_update_task_handle == NULL) {
        ESP_LOGW(TAG, "Sensor update task not running");
        return false;
    }

    ESP_LOGI(TAG, "Stopping sensor update task");

    vTaskDelete(sensor_update_task_handle);
    sensor_update_task_handle = NULL;
    update_interval_ms = 0;

    return true;
}

bool sensor_manager_register_callback(sensor_update_callback_t callback)
{
    if (callback == NULL) {
        ESP_LOGE(TAG, "Invalid callback function");
        return false;
    }

    if (callback_count >= MAX_CALLBACKS) {
        ESP_LOGE(TAG, "Maximum number of callbacks reached (%d)", MAX_CALLBACKS);
        return false;
    }

    callbacks[callback_count++] = callback;
    ESP_LOGI(TAG, "Registered sensor callback (%d/%d)", callback_count, MAX_CALLBACKS);

    return true;
}

bool sensor_manager_get_latest_reading(sensor_type_t type, sensor_reading_t *reading)
{
    if (reading == NULL || type > SENSOR_TYPE_VEHICLE_PRESENCE) {
        return false;
    }

    *reading = latest_readings[type];
    return reading->valid;
}