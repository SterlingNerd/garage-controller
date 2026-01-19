/*
 * Light Control Interface
 *
 * Defines the Home Assistant API operations for light control
 * Provides abstraction between Zigbee protocol and hardware drivers
 */

#ifndef LIGHT_CONTROL_H
#define LIGHT_CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_zigbee_core.h"

// Light endpoint configuration
#define LIGHT_DEFAULT_ENDPOINT 10

// Light control callback types
typedef void (*light_power_callback_t)(bool power);
typedef void (*light_brightness_callback_t)(uint8_t brightness);
typedef void (*light_color_callback_t)(uint8_t red, uint8_t green, uint8_t blue);

// Light state structure
typedef struct {
    bool power;
    uint8_t brightness;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} light_state_t;

// Light control operations
typedef struct {
    // Initialize light control system
    esp_err_t (*init)(void);

    // Set power state (on/off)
    esp_err_t (*set_power)(bool power);

    // Set brightness (0-255)
    esp_err_t (*set_brightness)(uint8_t brightness);

    // Set RGB color
    esp_err_t (*set_color)(uint8_t red, uint8_t green, uint8_t blue);

    // Get current light state
    esp_err_t (*get_state)(light_state_t *state);

    // Register callbacks for state changes
    esp_err_t (*register_power_callback)(light_power_callback_t callback);
    esp_err_t (*register_brightness_callback)(light_brightness_callback_t callback);
    esp_err_t (*register_color_callback)(light_color_callback_t callback);

    // Create Zigbee endpoint for this light
    esp_zb_ep_list_t* (*create_zigbee_endpoint)(uint8_t endpoint_id);
} light_control_ops_t;

// Get the light control operations interface
const light_control_ops_t* light_control_get_ops(void);

// Zigbee attribute handler for light control (called by zigbee_manager)
esp_err_t zb_light_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message);

#endif // LIGHT_CONTROL_H