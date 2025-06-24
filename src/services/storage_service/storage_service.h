#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void storage_init();
void record_rain_event();
void get_last_hour_rain(uint16_t current_time_minutes, uint8_t* total_rain);

#ifdef __cplusplus
}
#endif
