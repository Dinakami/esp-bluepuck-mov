#pragma once

#include "common.h"
#include "esp_log.h"

#if 1

#define MS_TO_HMS(total_ms,h,m,s,ms) do { \
        *h = (float)total_ms / 3600000.0f; \
        total_ms -= (*h * 3600000); \
        *m = (float)total_ms / 60000.0f; \
        total_ms -= (*m * 60000); \
        *s = (float)total_ms / 1000.0f; \
        total_ms -= (*s * 1000); \
        *ms = total_ms; \
    } while(0)

#define LOG_LEVEL_FORMAT_PRE(letter)     LOG_COLOR_ ## letter #letter LOG_RESET_COLOR " [" LOG_COLOR(LOG_COLOR_BLUE) "%-20.20s:%4d " LOG_RESET_COLOR "%02d:%02d:%02d ]: " LOG_COLOR_ ## letter
#define LOG_LEVEL_FORMAT_END()           LOG_RESET_COLOR "\n"
#define LOG_LEVEL_FORMAT(letter, format) LOG_LEVEL_FORMAT_PRE(letter) format LOG_LEVEL_FORMAT_END()

#ifdef ESP_LOG_LEVEL
#undef ESP_LOG_LEVEL
#endif

#define ESP_LOG_LEVEL(level, tag, format, ...) do { \
        int _h,_m,_s,_ms; \
        uint32_t _total_ms = esp_log_timestamp(); \
        MS_TO_HMS(_total_ms,&_h,&_m,&_s,&_ms); \
        if (level==ESP_LOG_ERROR )          { esp_log_write(ESP_LOG_ERROR,      tag, LOG_LEVEL_FORMAT(E, format), tag, __LINE__,_h,_m,_s, ##__VA_ARGS__); } \
        else if (level==ESP_LOG_WARN )      { esp_log_write(ESP_LOG_WARN,       tag, LOG_LEVEL_FORMAT(W, format), tag, __LINE__,_h,_m,_s, ##__VA_ARGS__); } \
        else if (level==ESP_LOG_DEBUG )     { esp_log_write(ESP_LOG_DEBUG,      tag, LOG_LEVEL_FORMAT(D, format), tag, __LINE__,_h,_m,_s, ##__VA_ARGS__); } \
        else if (level==ESP_LOG_VERBOSE )   { esp_log_write(ESP_LOG_VERBOSE,    tag, LOG_LEVEL_FORMAT(V, format), tag, __LINE__,_h,_m,_s, ##__VA_ARGS__); } \
        else                                { esp_log_write(ESP_LOG_INFO,       tag, LOG_LEVEL_FORMAT(I, format), tag, __LINE__,_h,_m,_s, ##__VA_ARGS__); } \
    } while(0)


#define LOGE(format,...) ESP_LOGE(__FILE_NAME__,format,##__VA_ARGS__)
#define LOGW(format,...) ESP_LOGW(__FILE_NAME__,format,##__VA_ARGS__)
#define LOGI(format,...) ESP_LOGI(__FILE_NAME__,format,##__VA_ARGS__)
#define LOGD(format,...) ESP_LOGD(__FILE_NAME__,format,##__VA_ARGS__)
#define LOGV(format,...) ESP_LOGV(__FILE_NAME__,format,##__VA_ARGS__)

#define LOG_PRINT_HEX(level, tag, data, data_len) do { \
        char _fmt[3*16+1] = {0}; \
        int _h,_m,_s,_ms; \
        uint32_t _total_ms = esp_log_timestamp(); \
        MS_TO_HMS(_total_ms,&_h,&_m,&_s,&_ms); \
        int _loops = (data_len) / 16; \
        _loops += ((data_len) % 16 == 0) ? 0 : 1; \
        int _index = 0; \
        for(int __i = 0; __i < _loops; ++__i) { \
            memset(_fmt,0,3*16+1); \
            int _print_len = MIN(16, (data_len) - _index); \
            for(int __j = 0; __j < _print_len; ++__j) { \
                sprintf(_fmt+strlen(_fmt), "%02X ", (data)[_index++]); \
            } \
            if (level==ESP_LOG_ERROR )          { esp_log_write(ESP_LOG_ERROR,   tag, LOG_LEVEL_FORMAT_PRE(E), tag, __LINE__,_h,_m,_s); } \
            else if (level==ESP_LOG_WARN )      { esp_log_write(ESP_LOG_WARN,    tag, LOG_LEVEL_FORMAT_PRE(W), tag, __LINE__,_h,_m,_s); } \
            else if (level==ESP_LOG_DEBUG )     { esp_log_write(ESP_LOG_DEBUG,   tag, LOG_LEVEL_FORMAT_PRE(D), tag, __LINE__,_h,_m,_s); } \
            else if (level==ESP_LOG_VERBOSE )   { esp_log_write(ESP_LOG_VERBOSE, tag, LOG_LEVEL_FORMAT_PRE(V), tag, __LINE__,_h,_m,_s); } \
            else                                { esp_log_write(ESP_LOG_INFO,    tag, LOG_LEVEL_FORMAT_PRE(I), tag, __LINE__,_h,_m,_s); } \
            esp_log_write(level, tag, _fmt); esp_log_write(level, tag, LOG_LEVEL_FORMAT_END()); \
        } \
    } while(0)

#define LOGE_HEX(data, data_len) LOG_PRINT_HEX(ESP_LOG_ERROR, __FILE_NAME__, data, data_len)
#define LOGW_HEX(data, data_len) LOG_PRINT_HEX(ESP_LOG_WARN,  __FILE_NAME__, data, data_len)
#define LOGI_HEX(data, data_len) LOG_PRINT_HEX(ESP_LOG_INFO,  __FILE_NAME__, data, data_len)
#define LOGD_HEX(data, data_len) LOG_PRINT_HEX(ESP_LOG_DEBUG, __FILE_NAME__, data, data_len)
#define LOGV_HEX(data, data_len) LOG_PRINT_HEX(ESP_LOG_ERROR, __FILE_NAME__, data, data_len)

#else
//disabled logs

#define LOGE(format,...) asm("nop")
#define LOGW(format,...) asm("nop")
#define LOGI(format,...) asm("nop")
#define LOGD(format,...) asm("nop")
#define LOGV(format,...) asm("nop")

#define LOGE_HEX(data, data_len) asm("nop")
#define LOGW_HEX(data, data_len) asm("nop")
#define LOGI_HEX(data, data_len) asm("nop")
#define LOGD_HEX(data, data_len) asm("nop")
#define LOGV_HEX(data, data_len) asm("nop")

#endif
