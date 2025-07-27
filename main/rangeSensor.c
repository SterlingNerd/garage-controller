#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "VL53L1X_api.h"
#include "dht.h"
#include "main.h"

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

void init_toF_sensor()
{
    ESP_LOGI(TAG, "Initializing ToF sensor...");

    // startup the I2C interface and scan for the devices
    i2c_init();

    // initalize all of the sensors
    VL53L1X_InitSensorArray(tof_array, sensor_count);

    // scanning should now show all devices at their new I2C address
    i2c_scan();

    // create the periodic time and service routine
    const esp_timer_create_args_t tof_sensor_timer_args = {
        .callback = &periodic_tof_sensor,
        .name = "tofsensor"};
    esp_timer_create(&tof_sensor_timer_args, &tof_sensor_timer);
    esp_timer_start_periodic(tof_sensor_timer, range_sensor_delay_ms * 1000);
}

