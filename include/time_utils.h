#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void time_init(void);
uint32_t get_current_minutes();
uint32_t get_elapsed_minutes_since_boot();

#ifdef __cplusplus
}
#endif
