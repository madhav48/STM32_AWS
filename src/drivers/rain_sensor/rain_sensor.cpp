#include "rain_sensor.h"
#include "services/storage_service/storage_service.h"
#include "stm32l0xx_hal_rtc.h"

void RainSensor::handlePulse() {     
    record_rain_event();
}
