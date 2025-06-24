#include "lux_sensor.h"

#define I2C_ADDR      0x4A    // 0x94 in 8-bit
#define MAX_RETRIES   3
#define MAX_ACK_WAIT_US 100  // Short timeout loop count

DFRobot_LUX_V30B::DFRobot_LUX_V30B(GPIO_TypeDef* sclPort, uint16_t sclPin,
                                   GPIO_TypeDef* sdaPort, uint16_t sdaPin,
                                   GPIO_TypeDef* enPort,  uint16_t enPin,
                                   TIM_HandleTypeDef* timer) {
    _sclPort = sclPort;
    _sclPin = sclPin;
    _sdaPort = sdaPort;
    _sdaPin = sdaPin;
    _enPort = enPort;
    _enPin = enPin;
    _timer = timer;
}

void DFRobot_LUX_V30B::delay_us(uint32_t us) {
    uint32_t start = __HAL_TIM_GET_COUNTER(_timer);
    while ((__HAL_TIM_GET_COUNTER(_timer) - start) < us);
}

void DFRobot_LUX_V30B::begin(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // SCL
    GPIO_InitStruct.Pin = _sclPin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(_sclPort, &GPIO_InitStruct);

    // SDA
    GPIO_InitStruct.Pin = _sdaPin;
    HAL_GPIO_Init(_sdaPort, &GPIO_InitStruct);

    // EN
    GPIO_InitStruct.Pin = _enPin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(_enPort, &GPIO_InitStruct);

    // Idle state high
    HAL_GPIO_WritePin(_sclPort, _sclPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(_sdaPort, _sdaPin, GPIO_PIN_SET);

    // Power cycle
    HAL_GPIO_WritePin(_enPort, _enPin, GPIO_PIN_RESET);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(_enPort, _enPin, GPIO_PIN_SET);

    // Wait until sensor responds with valid data..
    float lux = 0;
    int attempts = 0;
    while ((lightStrengthLux(&lux) != LUX_OK || lux <= 0.0f) && attempts++ < 10) {
        HAL_Delay(50);
    }
}

void DFRobot_LUX_V30B::iicStart(void) {
    HAL_GPIO_WritePin(_sdaPort, _sdaPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(_sclPort, _sclPin, GPIO_PIN_SET);
    delay_us(5);
    HAL_GPIO_WritePin(_sdaPort, _sdaPin, GPIO_PIN_RESET);
    delay_us(5);
    HAL_GPIO_WritePin(_sclPort, _sclPin, GPIO_PIN_RESET);
    delay_us(5);
}

void DFRobot_LUX_V30B::iicStop(void) {
    HAL_GPIO_WritePin(_sclPort, _sclPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(_sdaPort, _sdaPin, GPIO_PIN_RESET);
    delay_us(5);
    HAL_GPIO_WritePin(_sdaPort, _sdaPin, GPIO_PIN_SET);
    delay_us(5);
    HAL_GPIO_WritePin(_sclPort, _sclPin, GPIO_PIN_RESET);
    delay_us(5);
}

void DFRobot_LUX_V30B::sendAck(uint8_t ack) {
    GPIO_InitTypeDef gpio = { _sdaPin, GPIO_MODE_OUTPUT_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW };
    HAL_GPIO_Init(_sdaPort, &gpio);
    HAL_GPIO_WritePin(_sdaPort, _sdaPin, ack ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(_sclPort, _sclPin, GPIO_PIN_SET);
    delay_us(5);
    HAL_GPIO_WritePin(_sclPort, _sclPin, GPIO_PIN_RESET);
    delay_us(5);
}

int DFRobot_LUX_V30B::recvAck(void) {
    GPIO_InitTypeDef gpio = { _sdaPin, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW };
    HAL_GPIO_Init(_sdaPort, &gpio);

    HAL_GPIO_WritePin(_sclPort, _sclPin, GPIO_PIN_SET);
    delay_us(5);

    uint32_t timeout = MAX_ACK_WAIT_US;
    int ack = LUX_ERR_TIMEOUT;  // Default to timeout error
    while (timeout--) {
        if (HAL_GPIO_ReadPin(_sdaPort, _sdaPin) == GPIO_PIN_RESET) {
            ack = LUX_OK;  // Got ACK
            break;
        }
        delay_us(1);
    }

    HAL_GPIO_WritePin(_sclPort, _sclPin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(_sdaPort, _sdaPin, GPIO_PIN_SET);
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    HAL_GPIO_Init(_sdaPort, &gpio);
    delay_us(5);
    return ack;
}

int DFRobot_LUX_V30B::iicSend(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(_sdaPort, _sdaPin, (data & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        delay_us(5);
        HAL_GPIO_WritePin(_sclPort, _sclPin, GPIO_PIN_SET);
        delay_us(5);
        HAL_GPIO_WritePin(_sclPort, _sclPin, GPIO_PIN_RESET);
        delay_us(5);
        data <<= 1;
    }
    return recvAck();
}

uint8_t DFRobot_LUX_V30B::iicReadByte(void) {
    uint8_t data = 0;
    GPIO_InitTypeDef gpio = { _sdaPin, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW };
    HAL_GPIO_Init(_sdaPort, &gpio);

    for (uint8_t i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(_sclPort, _sclPin, GPIO_PIN_SET);
        delay_us(5);
        data |= HAL_GPIO_ReadPin(_sdaPort, _sdaPin);
        HAL_GPIO_WritePin(_sclPort, _sclPin, GPIO_PIN_RESET);
        delay_us(5);
        if (i < 7) data <<= 1;
    }

    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    HAL_GPIO_Init(_sdaPort, &gpio);
    return data;
}

int DFRobot_LUX_V30B::readBytes(uint8_t reg, uint8_t* buf, uint8_t len) {
    for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
        iicStart();
        int res;
        res = iicSend((I2C_ADDR << 1) | 0);
        if (res == LUX_ERR_TIMEOUT) return LUX_ERR_TIMEOUT;
        if (res != LUX_OK) continue;

        res = iicSend(reg);
        if (res == LUX_ERR_TIMEOUT) return LUX_ERR_TIMEOUT;
        if (res != LUX_OK) continue;

        iicStart();
        res = iicSend((I2C_ADDR << 1) | 1);
        if (res == LUX_ERR_TIMEOUT) return LUX_ERR_TIMEOUT;
        if (res != LUX_OK) continue;

        for (uint8_t i = 0; i < len; i++) {
            buf[i] = iicReadByte();
            sendAck((i == len - 1) ? 1 : 0);
        }

        iicStop();
        return LUX_OK;
    }
    return LUX_ERR_NO_RESPONSE;
}


int DFRobot_LUX_V30B::writeByte(uint8_t reg, uint8_t data) {
    iicStart();
    int res;
    res = iicSend((I2C_ADDR << 1) | 0);
    if (res != LUX_OK) return res;

    res = iicSend(reg);
    if (res != LUX_OK) return res;

    res = iicSend(data);
    if (res != LUX_OK) return res;

    iicStop();
    return LUX_OK;
}


int DFRobot_LUX_V30B::lightStrengthLux(float* luxOut) {
    uint8_t data[4];
    int res = readBytes(0x00, data, 4);
    if (res != LUX_OK) return res;

    uint32_t value = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
    *luxOut = ((float)value * 1.4f) / 1000.0f;
    return LUX_OK;
}


int DFRobot_LUX_V30B::lightStrengthRaw(uint32_t* rawOut) {
    uint8_t data[4];
    int res = readBytes(0x00, data, 4);
    if (res != LUX_OK) return res;

    *rawOut = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
    return LUX_OK;
}


void DFRobot_LUX_V30B::setMode(uint8_t manual, uint8_t CDR, uint8_t TIM) {
    uint8_t mode = (manual << 6) | (CDR << 3) | (TIM & 0x07);
    writeByte(0x04, mode);
}


uint8_t DFRobot_LUX_V30B::readMode(void) {
    uint8_t data = 0xFF;
    readBytes(0x04, &data, 1);  // Ignore return code for now
    return data;
}
