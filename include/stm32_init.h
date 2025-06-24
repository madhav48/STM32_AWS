#ifndef STM32_INIT_H
#define STM32_INIT_H

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32l0xx_hal.h"

extern RTC_HandleTypeDef hrtc;
extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart2;

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART2_UART_Init(void);
void MX_RTC_Init(void);
void MX_TIM2_Init(void);

#ifdef __cplusplus
}
#endif

#endif
