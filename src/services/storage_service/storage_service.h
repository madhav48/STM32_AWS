#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


// Error Codes...
#define STORAGE_OK               0
#define STORAGE_ERR_NULL_PTR     1


#define INVALID_MINUTE 0xFFFFFFFFU
typedef struct {
    uint32_t minute;  // Timestamp
    uint8_t count;    // Rain count
} RainEntry;


void storage_init();
void record_rain_event();
void get_last_hour_rain(uint32_t current_time_minutes, uint16_t* total_rain);
int get_rain_pulse_count(int* value);


#ifdef __cplusplus
}
#endif
