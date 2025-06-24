#include "stm32l0xx_hal.h"
#include <stdint.h>

// Error return codes
#define LUX_OK               0
#define LUX_ERR_TIMEOUT      1
#define LUX_ERR_NACK         2
#define LUX_ERR_NO_RESPONSE  3

class DFRobot_LUX_V30B {
public:
    DFRobot_LUX_V30B(GPIO_TypeDef* sclPort, uint16_t sclPin,
                     GPIO_TypeDef* sdaPort, uint16_t sdaPin,
                     GPIO_TypeDef* enPort,  uint16_t enPin,
                     TIM_HandleTypeDef* timer);

   
    void begin(void);  // Initializes GPIOs and powers on the sensor
   
    int lightStrengthLux(float* luxOut);  // Reads lux value (float), returns LUX_OK or error

    int lightStrengthRaw(uint32_t* rawOut); // Reads raw 32-bit lux value, returns LUX_OK or error

    void setMode(uint8_t manual, uint8_t CDR, uint8_t TIM);
    uint8_t readMode(void);

private:
    GPIO_TypeDef* _sclPort;
    uint16_t _sclPin;

    GPIO_TypeDef* _sdaPort;
    uint16_t _sdaPin;

    GPIO_TypeDef* _enPort;
    uint16_t _enPin;

    TIM_HandleTypeDef* _timer;

    void iicStart(void);
    void iicStop(void);
    void sendAck(uint8_t ack);
    int recvAck(void);
    int iicSend(uint8_t data);
    uint8_t iicReadByte(void);
    int readBytes(uint8_t reg, uint8_t* buf, uint8_t len);
    int writeByte(uint8_t reg, uint8_t data);
    void delay_us(uint32_t us);
};

