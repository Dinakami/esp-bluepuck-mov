#include "lightyear_bluetooth_shared.h"

uint8_t bluepuck_adv[31] = {};

void lightyear_bluetooth_init()
{
    // initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // initialize and enable bt controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        LOGE("Initialize bt controller failed. Err name: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        LOGE("Enable bt controller failed. Err name: %s", esp_err_to_name(ret));
        return;
    }

    // initialize and enable bt stack
    ret = esp_bluedroid_init();
    if (ret) {
        LOGE("Initialize bt stack failed. Err name: %s", esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        LOGE("Enable bt stack failed. Err name: %s", esp_err_to_name(ret));
        return;
    }

    // register the callback function to the gap module
    ret = esp_ble_gap_register_callback(esp_gap_cb);
    if (ret) {
        LOGE("Register gap callback failed. Err code: %x", ret);
        return;
    }

    // register the callback function to the gattc module
    ret = esp_ble_gattc_register_callback(esp_gattc_cb);
    if (ret) {
        LOGE("Register gattc callback failed. Err code: %x", ret);
        return;
    }

    // register gattc profiles
    ret = esp_ble_gattc_app_register(BLUEPUCK_MOV_PROFILE_ID);
    if (ret) {
        LOGE("Register gattc profile app failed. Err code: %x", ret);
        return;
    }

    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
    if (local_mtu_ret) {
        LOGE("Set local MTU failed. Err code: %x", ret);
    }
}

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    uint8_t* adv_name       = NULL;
    uint8_t adv_name_len    = 0;

    switch (event) {
        case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
            esp_ble_gap_start_scanning(SCAN_DURATION);
            break;
        }

        case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
            // indicate if scan start was successful or failed
            if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                LOGE("Scan start failed, error status = %x", param->scan_start_cmpl.status);
                break;
            }

            LOGI("BT scan start success\n");
            break;

        case ESP_GAP_BLE_SCAN_RESULT_EVT: {
            esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *) param;
            
            switch (scan_result->scan_rst.search_evt) {
                case ESP_GAP_SEARCH_INQ_RES_EVT: {
                    if (memcmp(remote_mov_v3_mac, scan_result->scan_rst.bda, 6) != 0) { // TODO: Is this a length comparison?
                        break;
                    }

#if CONFIG_EXAMPLE_DUMP_BDA_AND_ADV_NAME == true
                    LOGI_HEX(scan_result->scan_rst.bda, 6);
                    LOGI("Searched Adv Data Len %d, Scan Response Length %d", scan_result->scan_rst.adv_data_len, scan_result->scan_rst.scan_rsp_len);

                    adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv, ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
                    LOGI("Searched Device Name Len %d", adv_name_len);
                    if (adv_name != NULL) {
                        LOGI_HEX(adv_name, adv_name_len);
                        LOGI("Name: %.*s", adv_name_len, adv_name);
                    }                    
#if CONFIG_EXAMPLE_DUMP_ADV_DATA_AND_SCAN_RESP
                        if (scan_result->scan_rst.adv_data_len > 0) {
                            LOGI("adv data:");
                            LOGI_HEX(&scan_result->scan_rst.ble_adv[0], scan_result->scan_rst.adv_data_len);
                        }
                        if (scan_result->scan_rst.scan_rsp_len > 0) {
                            LOGI("scan resp:");
                            LOGI_HEX(&scan_result->scan_rst.ble_adv[scan_result->scan_rst.adv_data_len], scan_result->scan_rst.scan_rsp_len);
                        }
#endif
                        LOGI("\n");
#endif
                    
                    // copy last revceived adv packet
                    memcpy(bluepuck_adv, &scan_result->scan_rst.ble_adv[0], 31);
                    break;
                }

                case ESP_GAP_SEARCH_INQ_CMPL_EVT:
                    LOGI("Scan complete. Restarting Scan");
                    esp_ble_gap_start_scanning(SCAN_DURATION);
                    break;

                default:
                    break;
            }
            break;
        }

        default:
            break;

    }
}

static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    // if register event, store the gattc_if for each profile
    if (event == ESP_GATTC_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            gl_profile_tab[param->reg.app_id].gattc_if = gattc_if;
        } else {
            LOGI("Register profile gattc_if failed, app id %04x, status %d", param->reg.app_id, param->reg.status);
            return;
        }
    }

    /* If the gattc_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do {
        int idx;
        for (idx = 0; idx < NUM_OF_PROFILES; ++idx) {
            if (gattc_if == ESP_GATT_IF_NONE || gattc_if == gl_profile_tab[idx].gattc_if) {
                if (gl_profile_tab[idx].gattc_cb) {
                    gl_profile_tab[idx].gattc_cb(event, gattc_if, param);
                }
            }
        }
    } while (0);
}

static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;

    switch (event) {
        case ESP_GATTC_REG_EVT:
            LOGI("ESP_GATTC_REG_EVT");
            esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
            if (scan_ret) {
                LOGE("Setting scan params failed. Err code: %x", scan_ret);
            }
            break;

        default:
            break;
    }
}
