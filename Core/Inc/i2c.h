/*
 * i2c.h
 *
 *  Created on: Aug 6, 2021
 *      Author: vovav
 */

#ifndef INC_I2C_H_
#define INC_I2C_H_
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_gpio.h"

#define I2C_PULLUP (1<<0)
#define I2C_NOPULLUP (~(1<<0))
#define I2C_INPUT  (1<<1)
#define I2C_OUTPUT (~(1<<1))

#define I2C_DELAY_US 22

struct sPinPack{
	GPIO_TypeDef *GPIOx;
	uint16_t *GPIO_pin;
	uint8_t params;
};

struct Shandler{
	struct sPinPack scl,sda;
};

#define ACK 0
#define NACK 1

typedef struct Shandler I2C_handler;

void SFT_I2C_Init(GPIO_TypeDef *SCL_GPIOx, uint16_t *SCL_GPIO_pin,GPIO_TypeDef *SDA_GPIOx, uint16_t *SDA_GPIO_pin,I2C_handler *hi2c,uint8_t params);
void SFT_I2C_Master_Transmit(struct Shandler *si2c, uint8_t DevAddress, uint8_t *pData,uint16_t Size, uint8_t timeout);
void SFT_I2C_Master_Receive (struct Shandler *si2c, uint8_t DevAddress, uint8_t *pData,uint16_t Size, uint8_t timeout);



#endif /* INC_I2C_H_ */
