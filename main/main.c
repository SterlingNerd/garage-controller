/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier:  LicenseRef-Included
 *
 * Garage Door Controller Main Application
 *
 * This code is in the Public Domain (or CC0 licensed, at your option.)
 *
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_zb_light.h"

static const char *TAG = "GARAGE_CONTROLLER";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Garage Door Controller");

    // Initialize NVS
    ESP_ERROR_CHECK(nvs_flash_init());

    // Initialize Zigbee communication
    esp_zb_light_init();

    ESP_LOGI(TAG, "Garage Door Controller initialized");
}