/*
 * Garage Door Control Header
 *
 * Interface for controlling garage door mechanisms
 */

#ifndef DOOR_CONTROL_H
#define DOOR_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

// Door states
typedef enum {
    DOOR_STATE_UNKNOWN,
    DOOR_STATE_OPEN,
    DOOR_STATE_CLOSED,
    DOOR_STATE_OPENING,
    DOOR_STATE_CLOSING,
    DOOR_STATE_STOPPED
} door_state_t;

// Door commands
typedef enum {
    DOOR_CMD_OPEN,
    DOOR_CMD_CLOSE,
    DOOR_CMD_STOP,
    DOOR_CMD_TOGGLE
} door_command_t;

// Door control functions
bool door_control_init(void);
bool door_control_execute(door_command_t cmd);
door_state_t door_control_get_state(void);
bool door_control_is_moving(void);

#endif // DOOR_CONTROL_H