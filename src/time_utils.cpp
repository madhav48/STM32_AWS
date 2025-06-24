#include "time_utils.h"
#include "stm32l0xx_hal.h"

extern RTC_HandleTypeDef hrtc;

static uint32_t boot_minutes = 0;

static const uint8_t days_in_month[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    

static bool is_leap_year(uint16_t year)
{
    return ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)));
}


static void record_boot_time() {
    boot_minutes = get_current_minutes(); 
}


uint32_t get_current_minutes()
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

    uint32_t days = 0;

    uint16_t year = 2000 + date.Year;

    for (uint16_t y = 2000; y < year; ++y)
    {
        days += is_leap_year(y) ? 366 : 365;
    }

    for (uint8_t m = 1; m < date.Month; ++m)
    {
        days += days_in_month[m - 1];
        if (m == 2 && is_leap_year(year))
        {
            days += 1;
        }
    }

    days += (date.Date - 1);

    uint32_t total_minutes = days * 24 * 60 + time.Hours * 60 + time.Minutes;

    return total_minutes;
}


uint32_t get_elapsed_minutes_since_boot() {
    return get_current_minutes() - boot_minutes;
}


void time_init(void){
    record_boot_time();
}