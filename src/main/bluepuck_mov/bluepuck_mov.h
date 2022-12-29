#pragma once

#include "lightyear_bluetooth/lightyear_bluetooth_shared.h"

static uint8_t bluepuck_mov_alert_status_service[2] = {0x2A, 0x06};

static bool bluepuck_mov_in_motion = false;
static uint16_t num_motions = 0;

// TODO: Try connecting and chechking if adv data changes are still recorderd
// TODO: Use disconnect event to know when the puck is out of range. Keep scanning even after connect

void bluepuck_init();
void bluepuck_adv_handler(void * pvParameters);

static void mov_adv_handler();