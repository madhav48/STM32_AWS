#include "stm32l0xx_hal.h"

class RainSensor {
public:
    void handlePulse();        // To be called from EXTI ISR
};

