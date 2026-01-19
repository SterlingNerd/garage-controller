/*
 * Zigbee Manager
 *
 * Protocol-level Zigbee stack management
 * Handles network operations and delegates device-specific logic to managers
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "zigbee_manager.h"
#include "sensor_manager.h"
#include "light_control.h"

#if !defined ZB_ED_ROLE
#error Define ZB_ED_ROLE in idf.py menuconfig to compile light (End Device) source code.
#endif

static const char *TAG = "ZIGBEE_MANAGER";

// Sensor endpoint configuration
#define HA_ESP_SENSOR_ENDPOINT 0x02
/********************* Define functions **************************/
static void bdb_start_top_level_commissioning_cb(uint8_t mode_mask)
{
    ESP_RETURN_ON_FALSE(esp_zb_bdb_start_top_level_commissioning(mode_mask) == ESP_OK, , TAG, "Failed to start Zigbee commissioning");
}

void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    uint32_t *p_sg_p       = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig_type = *p_sg_p;
    switch (sig_type) {
    case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
        ESP_LOGI(TAG, "Initialize Zigbee stack");
        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
        break;
    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
        if (err_status == ESP_OK) {
            ESP_LOGI(TAG, "Device started up in %s factory-reset mode", esp_zb_bdb_is_factory_new() ? "" : "non");
            if (esp_zb_bdb_is_factory_new()) {
                ESP_LOGI(TAG, "Start network steering");
                esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
            } else {
                ESP_LOGI(TAG, "Device rebooted");
            }
        } else {
            /* commissioning failed */
            ESP_LOGW(TAG, "Failed to initialize Zigbee stack (status: %s)", esp_err_to_name(err_status));
        }
        break;
    case ESP_ZB_BDB_SIGNAL_STEERING:
        if (err_status == ESP_OK) {
            esp_zb_ieee_addr_t extended_pan_id;
            esp_zb_get_extended_pan_id(extended_pan_id);
            ESP_LOGI(TAG, "Joined network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN ID: 0x%04hx, Channel:%d, Short Address: 0x%04hx)",
                     extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                     extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                     esp_zb_get_pan_id(), esp_zb_get_current_channel(), esp_zb_get_short_address());
        } else {
            ESP_LOGI(TAG, "Network steering was not successful (status: %s)", esp_err_to_name(err_status));
            esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb, ESP_ZB_BDB_MODE_NETWORK_STEERING, 1000);
        }
        break;
    default:
        ESP_LOGI(TAG, "ZDO signal: %s (0x%x), status: %s", esp_zb_zdo_signal_to_string(sig_type), sig_type,
                 esp_err_to_name(err_status));
        break;
    }
}

static esp_err_t zb_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message)
{
    esp_err_t ret = ESP_OK;

    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)",
                        message->info.status);
    ESP_LOGI(TAG, "Received message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)", message->info.dst_endpoint, message->info.cluster,
             message->attribute.id, message->attribute.data.size);

    // Handle light attributes (endpoint 10)
    if (message->info.dst_endpoint == LIGHT_DEFAULT_ENDPOINT) {
        // Delegate to light manager's attribute handler
        return zb_light_attribute_handler(message);
    }

    // Sensor attributes are handled here (endpoint 2)
    // (sensor handling code would go here if needed)

    return ret;
}

static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message)
{
    esp_err_t ret = ESP_OK;
    switch (callback_id) {
    case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
        ret = zb_attribute_handler((esp_zb_zcl_set_attr_value_message_t *)message);
        break;
    default:
        ESP_LOGW(TAG, "Receive Zigbee action(0x%x) callback", callback_id);
        break;
    }
    return ret;
}

// Sensor update callback - called by sensor manager when new data is available
static void sensor_update_callback(const sensor_reading_t *reading)
{
    if (reading->type != SENSOR_TYPE_ENVIRONMENTAL || !reading->valid) {
        return;
    }

    // Convert temperature to Zigbee format (0.01°C resolution, signed)
    int16_t temp_zb = (int16_t)(reading->data.env.temperature_c * 100);

    // Convert humidity to Zigbee format (0.01% resolution, unsigned)
    uint16_t humidity_zb = (uint16_t)(reading->data.env.humidity_percent * 100);

    ESP_LOGI(TAG, "Updating Zigbee sensor values: Temp=%.2f°C (%d), Humidity=%.2f%% (%d)",
            reading->data.env.temperature_c, temp_zb,
            reading->data.env.humidity_percent, humidity_zb);

    // Update temperature measurement cluster
    esp_zb_lock_acquire(portMAX_DELAY);
    esp_zb_zcl_set_attribute_val(HA_ESP_SENSOR_ENDPOINT,
                               ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,
                               ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                               ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID,
                               &temp_zb,
                               false);
    esp_zb_lock_release();

    // Update humidity measurement cluster
    esp_zb_lock_acquire(portMAX_DELAY);
    esp_zb_zcl_set_attribute_val(HA_ESP_SENSOR_ENDPOINT,
                               ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
                               ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                               ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID,
                               &humidity_zb,
                               false);
    esp_zb_lock_release();
}

static void esp_zb_task(void *pvParameters)
{
    /* initialize Zigbee stack */
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZED_CONFIG();
    esp_zb_init(&zb_nwk_cfg);

    // Create light endpoint (light manager already initialized in main.c)
    const light_control_ops_t *light_ops = light_control_get_ops();
    esp_zb_ep_list_t *esp_zb_on_off_light_ep = light_ops->create_zigbee_endpoint(LIGHT_DEFAULT_ENDPOINT);

    // Create sensor endpoint with temperature and humidity clusters
    esp_zb_ep_list_t *esp_zb_sensor_ep = esp_zb_ep_list_create();

    // Create temperature measurement cluster
    esp_zb_temperature_meas_cluster_cfg_t temp_cfg = {
        .measured_value = ESP_ZB_ZCL_TEMP_MEASUREMENT_MEASURED_VALUE_DEFAULT,
        .min_value = ESP_ZB_ZCL_TEMP_MEASUREMENT_MIN_MEASURED_VALUE_DEFAULT,
        .max_value = ESP_ZB_ZCL_TEMP_MEASUREMENT_MAX_MEASURED_VALUE_DEFAULT,
    };
    esp_zb_attribute_list_t *temp_attr_list = esp_zb_temperature_meas_cluster_create(&temp_cfg);

    // Create humidity measurement cluster
    esp_zb_humidity_meas_cluster_cfg_t humidity_cfg = {
        .measured_value = ESP_ZB_ZCL_REL_HUMIDITY_MEASUREMENT_MEASURED_VALUE_DEFAULT,
        .min_value = ESP_ZB_ZCL_REL_HUMIDITY_MEASUREMENT_MIN_MEASURED_VALUE_DEFAULT,
        .max_value = ESP_ZB_ZCL_REL_HUMIDITY_MEASUREMENT_MAX_MEASURED_VALUE_DEFAULT,
    };
    esp_zb_attribute_list_t *humidity_attr_list = esp_zb_humidity_meas_cluster_create(&humidity_cfg);

    // Create cluster list for sensor endpoint
    esp_zb_cluster_list_t *sensor_cluster_list = esp_zb_zcl_cluster_list_create();

    // Add basic and identify clusters (required for all endpoints)
    esp_zb_basic_cluster_cfg_t basic_cfg = {
        .zcl_version = ESP_ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE,
        .power_source = ESP_ZB_ZCL_BASIC_POWER_SOURCE_BATTERY,
    };
    esp_zb_attribute_list_t *basic_attr_list = esp_zb_basic_cluster_create(&basic_cfg);
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_basic_cluster(sensor_cluster_list, basic_attr_list, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));

    esp_zb_identify_cluster_cfg_t identify_cfg = {
        .identify_time = 0,
    };
    esp_zb_attribute_list_t *identify_attr_list = esp_zb_identify_cluster_create(&identify_cfg);
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_identify_cluster(sensor_cluster_list, identify_attr_list, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));

    // Add temperature and humidity measurement clusters
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_temperature_meas_cluster(sensor_cluster_list, temp_attr_list, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_humidity_meas_cluster(sensor_cluster_list, humidity_attr_list, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));

    // Configure sensor endpoint
    esp_zb_endpoint_config_t sensor_ep_config = {
        .endpoint = HA_ESP_SENSOR_ENDPOINT,
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .app_device_id = ESP_ZB_HA_TEMPERATURE_SENSOR_DEVICE_ID,
        .app_device_version = 0,
    };
    ESP_ERROR_CHECK(esp_zb_ep_list_add_ep(esp_zb_sensor_ep, sensor_cluster_list, sensor_ep_config));

    zcl_basic_manufacturer_info_t info = {
        .manufacturer_name = "\x09""ESPRESSIF",      /* Customized manufacturer name */
        .model_identifier = "\x07""ESP32C6-GARAGE", /* Customized model identifier */
    };

    esp_zcl_utility_add_ep_basic_manufacturer_info(esp_zb_sensor_ep, HA_ESP_SENSOR_ENDPOINT, &info);

    // Register both endpoints
    esp_zb_device_register(esp_zb_on_off_light_ep);
    esp_zb_device_register(esp_zb_sensor_ep);

    esp_zb_core_action_handler_register(zb_action_handler);
    esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK);
    ESP_ERROR_CHECK(esp_zb_start(false));

    // Initialize and start sensor manager
    if (!sensor_manager_init()) {
        ESP_LOGE(TAG, "Failed to initialize sensor manager");
    } else {
        // Register callback for sensor updates
        sensor_manager_register_callback(sensor_update_callback);

        // Start periodic sensor updates (60 seconds)
        if (!sensor_manager_start_updates(60 * 1000)) {
            ESP_LOGE(TAG, "Failed to start sensor updates");
        }
    }

    esp_zb_stack_main_loop();
}

void zigbee_manager_init(void)
{
    esp_zb_platform_config_t config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };
    ESP_ERROR_CHECK(esp_zb_platform_config(&config));
    xTaskCreate(esp_zb_task, "Zigbee_main", 4096, NULL, 5, NULL);
}
