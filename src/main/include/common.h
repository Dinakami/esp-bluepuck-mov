#pragma once

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "esp_task_wdt.h"
#include "esp_ota_ops.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "console_log.h"

#define __FILE_NAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

// strings
#define STR_EMPTY(x)      (x == 0 || strlen(x) == 0)
#define STR_EQUAL(x,y)    (strncmp((x),(y),strlen((x))) == 0 && strlen(x) == strlen(y))

// delay milliseconds
#define delay(ms)    vTaskDelay(pdMS_TO_TICKS(ms))

#define MAX_TASK_PRIORITY (configMAX_PRIORITIES-1)

// math
#define MAX(a, b)      ((a) > (b) ? (a) : (b))
#define MIN(a, b)      ((a) < (b) ? (a) : (b))
#define ABS(a)         ((a) < 0 ? -1*(a) : (a))
#define SQUARE(a)      ((a) * (a))
#define BOUND(value,lbound,ubound)    (MIN(MAX((value), (lbound)), (ubound)))

// mac address macros
#define MAC_FMT "%02X:%02X:%02X:%02X:%02X:%02X"
#define MAC_LIST(mac) (mac)[0],(mac)[1],(mac)[2],(mac)[3],(mac)[4],(mac)[5]
#define MAC_IS_EMPTY(mac) ((mac)[0] == 0 && (mac)[1] == 0 && (mac)[2] == 0 && (mac)[3] == 0 && (mac)[4] == 0 && (mac)[5] == 0)

#define BINARY_LIST(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')
#define BINARY_FMT "%c%c%c%c%c%c%c%c"

#define BOOLSTR(b) (b) ? "true" : "false"


