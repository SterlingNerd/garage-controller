/*
 * Light Manager Implementation
 *
 * Manages Zigbee light endpoints and provides HA API abstraction
 * Integrates with hardware light drivers through callbacks
 */

#include "light_control.h"
#include "light_driver.h"
#include "esp_log.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "zcl_utility.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "LIGHT_MANAGER";

// Current light state
static light_state_t s_current_state = {
    .power = false,
    .brightness = 255,
    .red = 255,
    .green = 255,
    .blue = 255
};

// Callbacks
static light_power_callback_t s_power_callback = NULL;
static light_brightness_callback_t s_brightness_callback = NULL;
static light_color_callback_t s_color_callback = NULL;

// Mutex for thread safety
static SemaphoreHandle_t s_state_mutex = NULL;

// Forward declarations
static esp_err_t light_manager_init(void);
static esp_err_t light_manager_set_power(bool power);
static esp_err_t light_manager_set_brightness(uint8_t brightness);
static esp_err_t light_manager_set_color(uint8_t red, uint8_t green, uint8_t blue);
static esp_err_t light_manager_get_state(light_state_t *state);
static esp_err_t light_manager_register_power_callback(light_power_callback_t callback);
static esp_err_t light_manager_register_brightness_callback(light_brightness_callback_t callback);
static esp_err_t light_manager_register_color_callback(light_color_callback_t callback);
static esp_zb_ep_list_t* light_manager_create_zigbee_endpoint(uint8_t endpoint_id);

// Zigbee attribute handler for light control
esp_err_t zb_light_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message)
{
    esp_err_t ret = ESP_OK;
    bool power_state = s_current_state.power;
    uint8_t brightness = s_current_state.brightness;

    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG,
                        "Received message: error status(%d)", message->info.status);

    ESP_LOGI(TAG, "Light control - endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)",
             message->info.dst_endpoint, message->info.cluster, message->attribute.id, message->attribute.data.size);

    if (message->info.dst_endpoint == LIGHT_DEFAULT_ENDPOINT) {
        if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF) {
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID &&
                message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL) {
                power_state = message->attribute.data.value ? *(bool *)message->attribute.data.value : power_state;
                ESP_LOGI(TAG, "Light power command: %s", power_state ? "On" : "Off");

                // Update state and notify hardware
                if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE) {
                    s_current_state.power = power_state;
                    xSemaphoreGive(s_state_mutex);
                }

                light_manager_set_power(power_state);

                if (s_power_callback) {
                    s_power_callback(power_state);
                }
            }
        } else if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL) {
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID &&
                message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_U8) {
                brightness = *(uint8_t *)message->attribute.data.value;
                ESP_LOGI(TAG, "Light brightness command: %d", brightness);

                // Update state and notify hardware
                if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE) {
                    s_current_state.brightness = brightness;
                    xSemaphoreGive(s_state_mutex);
                }

                light_manager_set_brightness(brightness);

                if (s_brightness_callback) {
                    s_brightness_callback(brightness);
                }
            }
        } else if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_COLOR_CONTROL) {
            // Handle color control attributes (simplified - only RGB for now)
            ESP_LOGI(TAG, "Color control command received (not fully implemented)");
        }
    }

    return ret;
}

// Light control operations implementation
static esp_err_t light_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing light manager");

    // Create mutex for thread safety
    s_state_mutex = xSemaphoreCreateMutex();
    if (s_state_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create state mutex");
        return ESP_ERR_NO_MEM;
    }

    // Initialize hardware driver
    light_driver_init(s_current_state.power);

    ESP_LOGI(TAG, "Light manager initialized successfully");
    return ESP_OK;
}

static esp_err_t light_manager_set_power(bool power)
{
    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    s_current_state.power = power;

    // Update hardware
    light_driver_set_power(power);

    // Update Zigbee attribute
    esp_zb_lock_acquire(portMAX_DELAY);
    esp_zb_zcl_set_attribute_val(LIGHT_DEFAULT_ENDPOINT,
                               ESP_ZB_ZCL_CLUSTER_ID_ON_OFF,
                               ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                               ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
                               &power,
                               false);
    esp_zb_lock_release();

    xSemaphoreGive(s_state_mutex);
    return ESP_OK;
}

static esp_err_t light_manager_set_brightness(uint8_t brightness)
{
    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    s_current_state.brightness = brightness;

    // For now, map brightness to power state (0 = off, >0 = on)
    bool power = (brightness > 0);
    s_current_state.power = power;

    light_driver_set_power(power);

    // Update Zigbee attributes
    esp_zb_lock_acquire(portMAX_DELAY);

    // Update brightness attribute
    esp_zb_zcl_set_attribute_val(LIGHT_DEFAULT_ENDPOINT,
                               ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
                               ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                               ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID,
                               &brightness,
                               false);

    // Update on/off state based on brightness
    esp_zb_zcl_set_attribute_val(LIGHT_DEFAULT_ENDPOINT,
                               ESP_ZB_ZCL_CLUSTER_ID_ON_OFF,
                               ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                               ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
                               &power,
                               false);

    esp_zb_lock_release();

    xSemaphoreGive(s_state_mutex);
    return ESP_OK;
}

static esp_err_t light_manager_set_color(uint8_t red, uint8_t green, uint8_t blue)
{
    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    s_current_state.red = red;
    s_current_state.green = green;
    s_current_state.blue = blue;

    // TODO: Implement color control in hardware driver if supported
    ESP_LOGI(TAG, "Color set to RGB(%d, %d, %d) - not yet implemented in hardware", red, green, blue);

    xSemaphoreGive(s_state_mutex);
    return ESP_OK;
}

static esp_err_t light_manager_get_state(light_state_t *state)
{
    if (state == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    *state = s_current_state;
    xSemaphoreGive(s_state_mutex);

    return ESP_OK;
}

static esp_err_t light_manager_register_power_callback(light_power_callback_t callback)
{
    s_power_callback = callback;
    return ESP_OK;
}

static esp_err_t light_manager_register_brightness_callback(light_brightness_callback_t callback)
{
    s_brightness_callback = callback;
    return ESP_OK;
}

static esp_err_t light_manager_register_color_callback(light_color_callback_t callback)
{
    s_color_callback = callback;
    return ESP_OK;
}

static esp_zb_ep_list_t* light_manager_create_zigbee_endpoint(uint8_t endpoint_id)
{
    ESP_LOGI(TAG, "Creating Zigbee light endpoint %d", endpoint_id);

    // Create light endpoint with on/off, level control, and color control clusters
    esp_zb_on_off_light_cfg_t light_cfg = ESP_ZB_DEFAULT_ON_OFF_LIGHT_CONFIG();
    esp_zb_ep_list_t *light_ep = esp_zb_on_off_light_ep_create(endpoint_id, &light_cfg);

    if (light_ep == NULL) {
        ESP_LOGE(TAG, "Failed to create light endpoint");
        return NULL;
    }

    // Add manufacturer info
    zcl_basic_manufacturer_info_t info = {
        .manufacturer_name = "\x09""ESPRESSIF",      /* Customized manufacturer name */
        .model_identifier = "\x07""ESP32C6-GARAGE", /* Customized model identifier */
    };
    esp_zcl_utility_add_ep_basic_manufacturer_info(light_ep, endpoint_id, &info);

    // Attribute handler is registered in zigbee_manager.c

    ESP_LOGI(TAG, "Zigbee light endpoint %d created successfully", endpoint_id);
    return light_ep;
}

// Light control operations table
static const light_control_ops_t s_light_control_ops = {
    .init = light_manager_init,
    .set_power = light_manager_set_power,
    .set_brightness = light_manager_set_brightness,
    .set_color = light_manager_set_color,
    .get_state = light_manager_get_state,
    .register_power_callback = light_manager_register_power_callback,
    .register_brightness_callback = light_manager_register_brightness_callback,
    .register_color_callback = light_manager_register_color_callback,
    .create_zigbee_endpoint = light_manager_create_zigbee_endpoint,
};

const light_control_ops_t* light_control_get_ops(void)
{
    return &s_light_control_ops;
}