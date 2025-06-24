#include "services/storage_service/storage_service.h"
#include "config.h"
#include "stm32l0xx_hal.h"
#include <string.h>
#include "time_utils.h"

// EEPROM layout: 256 slots (4 bytes each)
static uint16_t current_eeprom_index = 0;
static uint32_t current_rain_total = 0;

// RAM buffer to store rain pulses per minute
static RainEntry rain_window[RAIN_WINDOW_SIZE_MIN];

// Convert index to EEPROM address
static uint32_t eeprom_addr(uint16_t index)
{
    return EEPROM_BASE_ADDR + (index % EEPROM_SLOT_COUNT) * RAIN_COUNTER_SIZE;
}

// Read 32-bit value from EEPROM
static uint32_t read_eeprom_uint32(uint32_t addr)
{
    return *(volatile uint32_t *)addr;
}

// Write 32-bit value to EEPROM safely
static HAL_StatusTypeDef write_eeprom_uint32(uint32_t addr, uint32_t value)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_StatusTypeDef status = HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, addr, value);
    HAL_FLASHEx_DATAEEPROM_Lock();
    return status;
}

// Find most recent valid slot in EEPROM
static void find_latest_eeprom_slot()
{
    uint32_t prev = 0;
    uint32_t curr = 0;
    current_eeprom_index = 0;

    for (uint16_t i = 0; i < EEPROM_SLOT_COUNT; ++i)
    {
        curr = read_eeprom_uint32(eeprom_addr(i));

        if (curr == 0xFFFFFFFF) break;
        if (curr >= prev)
        {
            prev = curr;
            current_eeprom_index = i;
        }
        else
        {
            break;
        }
    }

    current_rain_total = read_eeprom_uint32(eeprom_addr(current_eeprom_index));
}

// Initialize RAM buffer on boot
static void init_rain_ram_window()
{
    for (int i = 0; i < RAIN_WINDOW_SIZE_MIN; ++i)
    {
        rain_window[i].minute = INVALID_MINUTE;
        rain_window[i].count = 0;
    }
}


static void update_rain_ram_window(uint32_t current_time_minutes)
{
    uint16_t idx = current_time_minutes % RAIN_WINDOW_SIZE_MIN;

    if (rain_window[idx].minute == current_time_minutes)
    {
        rain_window[idx].count++;
    }
    else
    {
        rain_window[idx].minute = current_time_minutes;
        rain_window[idx].count = 1;
    }
}

extern "C" void storage_init()
{
    find_latest_eeprom_slot();
    init_rain_ram_window();
}

extern "C" void record_rain_event()
{
    current_rain_total++;

    // advance index
    current_eeprom_index = (current_eeprom_index + 1) % EEPROM_SLOT_COUNT;
    uint32_t addr = eeprom_addr(current_eeprom_index);
    write_eeprom_uint32(addr, current_rain_total);

    // last hour rain..
    update_rain_ram_window(get_current_minutes());
}

extern "C" int get_rain_pulse_count(int *value)
{
    if (value == nullptr)
    {
        return STORAGE_ERR_NULL_PTR;
    }

    *value = static_cast<int>(current_rain_total);
    return STORAGE_OK;
}

// Get last hour rain..
extern "C" void get_last_hour_rain(uint32_t current_time_minutes, uint8_t *total_rain)
{
    if (!total_rain)
        return;

    uint8_t sum = 0;
    for (int i = 0; i < RAIN_WINDOW_SIZE_MIN; ++i)
    {
        if (rain_window[i].minute != INVALID_MINUTE &&
            (current_time_minutes - rain_window[i].minute) <= 60)
        {
            sum += rain_window[i].count;
        }
    }
    *total_rain = sum;
}
