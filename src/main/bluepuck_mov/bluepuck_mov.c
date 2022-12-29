 #include "bluepuck_mov.h"

uint8_t last_adv_data[31] = {0};

void bluepuck_init()
{
    lightyear_bluetooth_init();
}

void bluepuck_adv_handler(void * pvParameters)
{   
    for (;;) {
        // run for every new adv packet
        if ( ((sizeof(bluepuck_adv) / sizeof(bluepuck_adv)[0]) != 0) && (memcmp(bluepuck_adv, last_adv_data, 31) != 0) ) {
            mov_adv_handler();
        }
        memcpy(last_adv_data, bluepuck_adv, 31);

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void mov_adv_handler() {
    uint8_t alert_status_service[2] = {bluepuck_adv[6], bluepuck_adv[5]}; // ``big eldian

    if (memcmp(alert_status_service, bluepuck_mov_alert_status_service, 2) == 0) {
        // LOGI("Alert status service:");
        // LOGI_HEX(alert_status_service, 2);

        // Move data from adv packet
        uint8_t raw_data[2] = {bluepuck_adv[7], bluepuck_adv[8]}; // little eldian
        uint16_t *mov_data = (uint16_t*)raw_data; // big eldian
        // LOGI("MOV Data: %d, 0x%04x", *mov_data, *mov_data);
        
        // Number of motions
        num_motions = *mov_data >> 1;
        // LOGI("Number of motions = %d", num_motions);
        
        // Instantenuous motion
        uint8_t inst_motion = (raw_data[0] & 0x01);
        // LOGI("Instantaneous data = %d\n", inst_motion);

        if (inst_motion == 1) {
            LOGI("Puck active");
        } else {
            LOGI("Pudk idle. No motion detected");
            LOGI("New number of motions = %d", num_motions);
        }
    }
}
