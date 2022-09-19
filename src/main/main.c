#include "common.h"
#include "bluepuck_mov/bluepuck_mov.h"

// ------------------------------------------------------------------------------------------

const char* last_reset_reason_str();

// ------------------------------------------------------------------------------------------


void app_main(void)
{
    LOGI("IDF Version:       %d.%d.%d (%s)", ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR, ESP_IDF_VERSION_PATCH, esp_get_idf_version());
    LOGI("Last Reset Reason: (%d) %s", esp_reset_reason(), last_reset_reason_str());
    const esp_app_desc_t* app_desc = esp_ota_get_app_description();
    LOGI("APP Version:       %s", app_desc->version);
    LOGI("Project Name:      %s", app_desc->project_name);
    LOGI("Compile Time:      %s", app_desc->time);
    LOGI("Compile Date:      %s", app_desc->date);

    bluepuck_mov_init();
}

const char* last_reset_reason_str()
{
    switch(esp_reset_reason())
    { 
        case ESP_RST_UNKNOWN:   return "UNKNOWN";   //Reset reason can not be determined
        case ESP_RST_POWERON:   return "POWERON";   //Reset due to power-on event (occurs after flashing)
        case ESP_RST_EXT:       return "EXT";       //Reset by external pin (not applicable for ESP32)
        case ESP_RST_SW:        return "SW";        //Software reset via esp_restart
        case ESP_RST_PANIC:     return "PANIC";     //Software reset due to exception/panic
        case ESP_RST_INT_WDT:   return "INT_WDT";   //Reset (software or hardware) due to interrupt watchdog
        case ESP_RST_TASK_WDT:  return "TASK_WDT";  //Reset due to task watchdog
        case ESP_RST_WDT:       return "WDT";       //Reset due to other watchdogs (occurs with Ctrl+T+R while monitoring)
        case ESP_RST_DEEPSLEEP: return "DEEPSLEEP"; //Reset after exiting deep sleep mode
        case ESP_RST_BROWNOUT:  return "BROWNOUT";  //Brownout reset (software or hardware)
        case ESP_RST_SDIO:      return "SDIO";      //Reset over SDIO
    }
    return "UNKNOWN";
}

