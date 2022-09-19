#pragma once

#include "nvs.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_common_api.h"

#include "common.h"

void lightyear_bluetooth_init();
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

#define CONFIG_EXAMPLE_DUMP_ADV_DATA_AND_SCAN_RESP true

#define NUM_OF_PROFILES 1
#define BLUEPUCK_MOV_PROFILE_ID 0

#define REMOTE_SERVICE_UUID     0x00FF

static const char remote_device_name[] = "LY BluePuckMov remove\0";
static bool connect     = false;
static bool get_server  = false;

struct gattc_profile_inst {
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t app_id;
    uint16_t conn_id;
    esp_bd_addr_t remote_bda;
    uint16_t service_start_handle;
    uint16_t service_end_handle;
};

/* This array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gattc_profile_inst gl_profile_tab[NUM_OF_PROFILES] = {
    [BLUEPUCK_MOV_PROFILE_ID] = {
        .gattc_cb = gattc_profile_event_handler,
        .gattc_if = ESP_GATT_IF_NONE    // No gatt_if yet, so initial is ESP_GATT_IF_NONE
    }
};

static esp_ble_scan_params_t ble_scan_params = {
    .scan_type          = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval      = 0x50,
    .scan_window        = 0x30,
    .scan_duplicate     = BLE_SCAN_DUPLICATE_DISABLE
};

static esp_bt_uuid_t remote_filter_service_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid = {.uuid16 = REMOTE_SERVICE_UUID}
};
