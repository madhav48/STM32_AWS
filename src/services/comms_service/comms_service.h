#ifndef COMMS_SERVICE_H
#define COMMS_SERVICE_H

#include "stm32l0xx_hal.h"

void Comms_Init(UART_HandleTypeDef* huart);
void Comms_Handle();        
void Comms_Resume();       

#endif