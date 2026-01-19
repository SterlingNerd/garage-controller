/*
 * Sensor Manager Header
 *
 * Manages periodic sensor readings and provides interface for sensor data
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "sensor_interface.h"

// Sensor update callback type
typedef void (*sensor_update_callback_t)(const sensor_reading_t *reading);

// Sensor manager functions
bool sensor_manager_init(void);
bool sensor_manager_start_updates(uint32_t interval_ms);
bool sensor_manager_stop_updates(void);
bool sensor_manager_register_callback(sensor_update_callback_t callback);
bool sensor_manager_get_latest_reading(sensor_type_t type, sensor_reading_t *reading);

#endif // SENSOR_MANAGER_H