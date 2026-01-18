/*
 * Safety Monitor Header
 *
 * Safety systems and interlocks for garage door operation
 */

#ifndef SAFETY_MONITOR_H
#define SAFETY_MONITOR_H

#include <stdbool.h>

// Safety check results
typedef enum {
    SAFETY_OK,
    SAFETY_BLOCKED,
    SAFETY_SENSOR_ERROR,
    SAFETY_DOOR_STUCK,
    SAFETY_EMERGENCY_STOP
} safety_status_t;

// Safety monitor functions
bool safety_monitor_init(void);
safety_status_t safety_check_before_open(void);
safety_status_t safety_check_before_close(void);
bool safety_emergency_stop(void);
bool safety_is_emergency_active(void);

#endif // SAFETY_MONITOR_H