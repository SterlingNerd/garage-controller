#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "VL53L1X_api.h"
#include "VL53L1X_error_codes.h"
#include "dht.h"

/**
 * Time-of-Flight (ToF) Sensor
 *
 *
 *
 *
 */
#define garage_door_sensor_index 0
#define garage_door_sensor_shutdown_gpio_num 9
#define car_sensor_index 1
#define car_sensor_shutdown_gpio_num 18

#define range_sensor_cycle_ms (20) // the timing budget for the VL53L1
#define range_sensor_delay_ms (25) // periodic timer for measurements, at least 5 + MEASUREMENT_CYCLE_MS

static VL53L1_Dev_t tof_array[] = {
    {// [0]
     .I2cDevAddr = VL53L1_I2C_ADDRESS + 2,
     .shutdown_pin = garage_door_sensor_shutdown_gpio_num,
     .interrupt_pin = 0,
     .distance_mode = DISTANCE_MODE_SHORT,
     .timing_budget = range_sensor_cycle_ms,
     .inter_measurement = range_sensor_delay_ms},
    // {// [1]
    //  .I2cDevAddr = VL53L1_I2C_ADDRESS + 4,
    //  .shutdown_pin = car_sensor_shutdown_gpio_num,
    //  .interrupt_pin = 0,
    //  .distance_mode = DISTANCE_MODE_SHORT,
    //  .timing_budget = range_sensor_cycle_ms,
    //  .inter_measurement = range_sensor_delay_ms}
};
uint8_t sensor_count = sizeof(tof_array) / sizeof(VL53L1_Dev_t);
esp_timer_handle_t tof_sensor_timer; // collects measurements from the ToF sensor

static void periodic_tof_sensor(void *arg)
{
    // mark the cycle time for this service routine
    int64_t update_measurement_time = esp_timer_get_time();

    for (int k = 0; k < sensor_count; k++)
    {
        VL53L1_Dev_t *tof = &tof_array[k];
        // get the measurement and start a new measurement cycle
        tof->range_error = VL53L1X_GetAndRestartMeasurement(tof->I2cDevAddr, &tof->range_status, &tof->range_mm);
        tof->cycle_time = update_measurement_time - tof->time_stamp;
        tof->time_stamp = update_measurement_time;
    }
}

int init_tof_sensor()
{
    // startup the I2C interface
    ESP_LOGI("init_tof_sensor", "Enabling pull-up resistors on i2c pins");
    gpio_pullup_en(6); // Enable pull-up resistor if needed
    gpio_pullup_en(7); // Enable pull-up resistor if needed
    gpio_set_level(garage_door_sensor_shutdown_gpio_num, 1);

    // startup the I2C interface
    ESP_LOGI("init_tof_sensor", "Initializing i2c interface");
    i2c_init();

    // scanning should now show all devices at their new I2C address
    ESP_LOGI("init_tof_sensor", "#1 Scanning i2c devices...");
    i2c_scan();

    // initalize all of the sensors
    ESP_LOGI("init_tof_sensor", "Initializing Tof sensor...");
    VL53L1X_ERROR error = VL53L1X_InitSensorArray(tof_array, sensor_count);
    if (error != VL53L1_ERROR_NONE)
    {
        ESP_LOGE("init_tof_sensor", "Sensor array init failed: %d", error);
        return error;
    }

    // scanning should now show all devices at their new I2C address
    ESP_LOGI("init_tof_sensor", "#2 Scanning i2c devices...");
    i2c_scan();

    // create the periodic time and service routine
    ESP_LOGI("init_tof_sensor", "Starting tof timer...");
    const esp_timer_create_args_t tof_sensor_timer_args = {
        .callback = &periodic_tof_sensor,
        .name = "tofsensor"};
    esp_timer_create(&tof_sensor_timer_args, &tof_sensor_timer);
    esp_timer_start_periodic(tof_sensor_timer, range_sensor_delay_ms * 1000);

    return 0;
}

void read_door_range()
{
    ESP_LOGI("read_door_range", "Reading door range...");
    for (int i = 0; i < sensor_count; i++)
    {
        VL53L1_Dev_t *tof = &tof_array[i];
        ESP_LOGI("  read_door_range", "Sensor %d: Range: %d mm, Status: %d, Error:%d", i, tof->range_mm, tof->range_status, tof->range_error);
    }
}

/**
 * DHT (Digital Humidity and Temperature) Sensor
 *
 *
 *
 *
 *
 */
#define dht_sensor_type_t DHT_TYPE_AM2301
#define dht_sensor_gpio_num 0
#define dht_sensor_pullup_enabled false

void init_dht_sensor()
{
    ESP_LOGI("init_dht_sensor", "Initializing DHT sensor...");

    // Enable internal pull-up resistor if specified in Kconfig
    // gpio_pullup_en(gpio_num);
    if (dht_sensor_pullup_enabled)
    {
        gpio_pullup_en(dht_sensor_gpio_num); // Enable pull-up resistor if needed
    }
    else
    {
        gpio_pullup_dis(dht_sensor_gpio_num); // Disable pull-up resistor if not needed
    }
}

void read_temp(float *humidity, float *temperature)
{
    ESP_LOGI("read_temp", "Reading dht sensor...");
    esp_err_t result = dht_read_float_data(dht_sensor_type_t, dht_sensor_gpio_num, humidity, temperature);
    if (result == ESP_OK)
    {
        ESP_LOGI("  read_temp", "Humidity: %.1f%% Temperature: %.1f°C, %.1f°F", *humidity, *temperature, *temperature * 9.0 / 5.0 + 32);
    }
    else
    {
        ESP_LOGE("  read_temp", "Failed to read sensor data: %s", esp_err_to_name(result));
    }
}

/**
 * Main App
 *
 *
 *
 *
 */

int init()
{
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    ESP_LOGI("init", "Initializing...");
    init_dht_sensor();
    return init_tof_sensor();
}

void app_main(void)
{
    ESP_LOGI("app_main", "Starting Garage Sensor");
    int error = init();
    if (error != 0)
    {
        ESP_LOGE("app_main", "Initialization failed with error: %d", error);
        return;
    }

    float humidity = 0, temperature = 0;

    while (true)
    {
        read_temp(&humidity, &temperature);
        read_door_range();

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
