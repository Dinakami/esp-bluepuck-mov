#include "lightyear_bluetooth_client.h"

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
            uint32_t duration = 30;     // in seconds
            esp_ble_gap_start_scanning(duration);
            break;
        }

        case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
            // indicaate if scan start was successful or failed
            if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                LOGE("Scan start failed, error status = %x", param->scan_start_cmpl.status);
                break;
            }

            LOGI("BT scan start success");
            break;

        case ESP_GAP_BLE_SCAN_RESULT_EVT: {
            esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *) param;
            
            switch (scan_result->scan_rst.search_evt) {
                case ESP_GAP_SEARCH_INQ_RES_EVT: {
                    uint8_t mac[6] = {0xD8, 0x05, 0xA9, 0xC5, 0x7C, 0x4F};
                    if (memcmp(mac, scan_result->scan_rst.bda, 6) != 0) { // TODO: Is this a length comparison?
                        break;
                    }

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

                    // Connect to remote device if name
                    if (adv_name != NULL) {
                        if (strlen(remote_device_name) == adv_name_len &&
                            strncmp((char *) adv_name, remote_device_name, adv_name_len) == 0) {
                            LOGI("Searched device %s\n", remote_device_name);

                            if (connect == false) {
                                connect = true;
                                LOGI("Connected to the remote device.");

                                esp_ble_gap_stop_scanning();
                                esp_ble_gattc_open(
                                    gl_profile_tab[BLUEPUCK_MOV_PROFILE_ID].gattc_if, scan_result->scan_rst.bda,
                                    scan_result->scan_rst.ble_addr_type, true
                                );
                            }
                        }               
                    }
                    break;
                }

                case ESP_GAP_SEARCH_INQ_CMPL_EVT:
                    break;

                default:
                    break;
            }
            break;
        }

        case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
            if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                LOGE("Scan stop failed, error status = %x", param->scan_stop_cmpl.status);
                break;
            }
            LOGI("Stop scan Successful");
            break;

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

        case ESP_GATTC_OPEN_EVT:
            if (param->open.status != ESP_GATT_OK) {
                LOGE("Gattc open conn failed, status %d", p_data->open.status);
                break;
            }
            LOGI("Open success");
            break;

        case ESP_GATTC_CONNECT_EVT: {
            LOGI("ESP_GATTC_CONNECT_EVT. conn_id %d, gattc_if %d", p_data->connect.conn_id, gattc_if);
            gl_profile_tab[BLUEPUCK_MOV_PROFILE_ID].conn_id = p_data->connect.conn_id;
            memcpy(gl_profile_tab[BLUEPUCK_MOV_PROFILE_ID].remote_bda, p_data->connect.remote_bda, sizeof(esp_bd_addr_t));
            LOGI("REMOTE BDA:");
            LOGI_HEX(gl_profile_tab[BLUEPUCK_MOV_PROFILE_ID].remote_bda, sizeof(esp_bd_addr_t));

            esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req(gattc_if, p_data->connect.conn_id);
            if (mtu_ret) {
                LOGE("MTU request error. Error code = %x", mtu_ret);
            }
            break;
        }

        case ESP_GATTC_CFG_MTU_EVT:
            if (param->cfg_mtu.status != ESP_GATT_OK) {
                LOGE("Config mtu request failed, error status = %x", param->cfg_mtu.status);
                break;
            }
            LOGI("ESP_GATTC_CFG_MTU_EVT, Status %d, MTU %d, conn_id %d", param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
            break;

        // case ESP_GATTC_DIS_SRVC_CMPL_EVT:
        //     if (param->dis_srvc_cmpl.status != ESP_GATT_OK) {
        //         LOGE("Discover service failed, status %d", param->dis_srvc_cmpl.status);
        //         break;
        //     }

        //     LOGI("Discover service complete conn_id %d", param->dis_srvc_cmpl.conn_id);
        //     esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &remote_filter_service_uuid);
        //     break;

        // case ESP_GATTC_SEARCH_RES_EVT: {
        //     LOGI("SEARCH RES: conn_id = %x is primary service %d", p_data->search_res.conn_id, p_data->search_res.is_primary);
        //     LOGI("Start handle %d end handle %d current handle value %d", p_data->search_res.start_handle, p_data->search_res.end_handle, p_data->search_res.srvc_id.inst_id);

        //     if (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16 &&
        //         p_data->search_res.srvc_id.uuid.uuid.uuid16 == REMOTE_SERVICE_UUID) {
        //             LOGI("SERVICE FOUND");
        //             get_server = true;
        //             gl_profile_tab[BLUEPUCK_MOV_PROFILE_ID].service_start_handle = p_data->search_res.start_handle;
        //             gl_profile_tab[BLUEPUCK_MOV_PROFILE_ID].service_end_handle = p_data->search_res.end_handle;
        //             LOGI("UUID16: %x", p_data->search_res.srvc_id.uuid.uuid.uuid16);
        //         }
        //         break;
        // }

        // // TODO: Complete
        // case ESP_GATTC_SEARCH_CMPL_EVT:
        //     if (p_data->search_cmpl.status != ESP_GATT_OK) {
        //         LOGE("Search service failed, error status = %x", p_data->search_cmpl.status);
        //         break;
        //     }

        //     if (p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_REMOTE_DEVICE) {
        //         LOGI("Get service information from remote device");
        //     } else if (p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_NVS_FLASH) {
        //         LOGI("Get service information from flash");
        //     } else {
        //         LOGI("Unknown service source");
        //     }
        //     LOGI("ESP_GATTC_SEARCH_CMPL_EVT");

        //     // if (get_server) {

        //     // }

        //     break;

        case ESP_GATTC_DISCONNECT_EVT:
            connect = false;
            get_server = false;
            LOGI("ESP_GATTC_DISCONNECT_EVT, reason = %d", p_data->disconnect.reason);
            break;

        default:
            break;
    }
}
