#include "comms_service.h"
#include "drivers/lux_sensor/lux_sensor.h"
#include "config.h"
#include <string.h>
#include <stdio.h>

extern UART_HandleTypeDef huart2;
extern DFRobot_LUX_V30B luxSensor;
extern RTC_HandleTypeDef hrtc;

static uint8_t rxByte = 0;
static char uartBuffer[32];
static uint8_t uartIndex = 0;
static bool commandReady = false;

void Comms_Init(UART_HandleTypeDef* huart) {
    HAL_UART_Receive_IT(huart, &rxByte, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        if (rxByte == '\n') {
            uartBuffer[uartIndex] = '\0';
            commandReady = true;
            uartIndex = 0;
        } else {
            if (uartIndex < sizeof(uartBuffer) - 1) {
                uartBuffer[uartIndex++] = rxByte;
            }
        }
        HAL_UART_Receive_IT(&huart2, &rxByte, 1);
    }
}

void Comms_Handle() {
    if (!commandReady) return;
    commandReady = false;

    char response[64] = {0};

    if (strcmp(uartBuffer, "GET_LUX") == 0) {
        float lux;
        if (luxSensor.lightStrengthLux(&lux) == LUX_OK) {
            snprintf(response, sizeof(response), "LUX: %.2f\n", lux);
        } else {
            snprintf(response, sizeof(response), "LUX: ERROR\n");
        }

    }
    
    // Add for rain..

    else {
        snprintf(response, sizeof(response), "ERR: INVALID\n");
    }

    HAL_UART_Transmit(&huart2, (uint8_t*)response, strlen(response), HAL_MAX_DELAY);
}

void Comms_Resume() {
    uartIndex = 0;
    commandReady = false;
    HAL_UART_Receive_IT(&huart2, &rxByte, 1);
}
