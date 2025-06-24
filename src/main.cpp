#include "drivers/lux_sensor/lux_sensor.h"
#include "config.h"
#include <stdio.h>
#include "stm32l0xx_hal.h"
#include "drivers/rain_sensor/rain_sensor.h"
#include "services/comms_service/comms_service.h"
#include "services/storage_service/storage_service.h"
#include "time_utils.h"


extern "C"{
    #include "stm32_init.h"
}


extern TIM_HandleTypeDef LUX_TIMER_HANDLE;
extern UART_HandleTypeDef huart2;
extern RTC_HandleTypeDef hrtc;

DFRobot_LUX_V30B luxSensor(LUX_SCL_GPIO_Port, LUX_SCL_Pin,
                           LUX_SDA_GPIO_Port, LUX_SDA_Pin,
                           LUX_EN_GPIO_Port,  LUX_EN_Pin,
                           &LUX_TIMER_HANDLE);

RainSensor rainSensor;





void enter_stop_mode(void) {
    HAL_SuspendTick(); 
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    HAL_ResumeTick();   
    SystemClock_Config();         // Re-init clocks
    MX_GPIO_Init();               // Optional if all EXTI preserved
    MX_USART2_UART_Init();        // Re-init UART after STOP
    Comms_Resume();               // Resume reception
}

int main(void) {

    HAL_Init();
    SystemClock_Config();        // Clock setup
    MX_GPIO_Init();              // Base GPIO init
    MX_TIM2_Init();            
    MX_RTC_Init(); 
    MX_USART2_UART_Init();

    ENABLE_LUX_GPIO_CLOCKS();    // Enable GPIO clocks used by sensor
    HAL_TIM_Base_Start(&LUX_TIMER_HANDLE);  // Start timer for delay_us()
    luxSensor.begin();           // Initialize sensor

    Comms_Init(&huart2);
    storage_init();
    time_init();


    // float lux = 0.0f;

    while (1) {

        /* Get the lux data like this ...

        int result = luxSensor.lightStrengthLux(&lux);

        if (result == LUX_OK) {
            printf("Lux: %.2f lx\r\n", lux);
        } else if (result == LUX_ERR_TIMEOUT) {
            printf("Lux Read Error: Timeout\r\n");
        } else if (result == LUX_ERR_NO_RESPONSE) {
            printf("Lux Read Error: No response\r\n");
        } else if (result == LUX_ERR_NACK) {
            printf("Lux Read Error: NACK\r\n");
        } else {
            printf("Lux Read Error: Unknown\r\n");
        }

        HAL_Delay(1000);  // Read every 1s

        */

        enter_stop_mode();
        // MCU will stop here until EXTI interrupt..
    }
}