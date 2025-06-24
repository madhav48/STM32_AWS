#include "stm32l0xx_hal.h"

// Lux Sensor Pin Definitions
#define LUX_SCL_GPIO_Port     GPIOA
#define LUX_SCL_Pin           GPIO_PIN_1

#define LUX_SDA_GPIO_Port     GPIOA
#define LUX_SDA_Pin           GPIO_PIN_2

#define LUX_EN_GPIO_Port      GPIOA
#define LUX_EN_Pin            GPIO_PIN_3

#define ENABLE_LUX_GPIO_CLOCKS() do { \
    __HAL_RCC_GPIOA_CLK_ENABLE(); \
} while(0)

// Timer used for microsecond delays..
#define LUX_TIMER_HANDLE      htim2

// Rain Sensor (Reed Switch) Configuration
#define RAIN_GPIO_Port      GPIOA
#define RAIN_Pin            GPIO_PIN_4
#define RAIN_EXTI_IRQn      EXTI4_15_IRQn


// Storage Config 

#define EEPROM_BASE_ADDR        0x08080000U
#define EEPROM_SLOT_COUNT       256          
#define RAIN_COUNTER_SIZE       4    
#define EEPROM_TOTAL_BYTES      (EEPROM_SLOT_COUNT * RAIN_COUNTER_SIZE)

#define RAIN_WINDOW_SIZE_MIN    60           // 1 hour window