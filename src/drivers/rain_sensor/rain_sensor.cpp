#include "rain_sensor.h"
#include "services/storage_service/storage_service.h"


void RainSensor::handlePulse() {
    record_rain_event();        // Pass to storage
}
