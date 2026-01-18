/*
 * Sensor Interface Header
 *
 * Generic interface for all sensor types in the garage controller
 */

#ifndef SENSOR_INTERFACE_H
#define SENSOR_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

// Sensor types
typedef enum {
    SENSOR_TYPE_DEPTH,
    SENSOR_TYPE_ENVIRONMENTAL,
    SENSOR_TYPE_VEHICLE_PRESENCE
} sensor_type_t;

// Generic sensor data structure
typedef struct {
    sensor_type_t type;
    uint32_t timestamp;
    bool valid;
    union {
        float distance_mm;        // For depth sensors
        struct {                 // For environmental sensors
            float temperature_c;
            float humidity_percent;
        } env;
        bool vehicle_present;    // For vehicle presence
    } data;
} sensor_reading_t;

// Sensor interface functions
bool sensor_init(void);
bool sensor_read(sensor_type_t type, sensor_reading_t *reading);
bool sensor_is_available(sensor_type_t type);

#endif // SENSOR_INTERFACE_H