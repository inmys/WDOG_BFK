/*
 * i2c.c
 *
 *  Created on: Aug 6, 2021
 *      Author: vovav
 */

#include "i2c.h"

#define CLOCK_UP HAL_GPIO_WritePin(si2c->scl.GPIOx,si2c->scl.GPIO_pin,1)
#define CLOCK_DOWN HAL_GPIO_WritePin(si2c->scl.GPIOx,si2c->scl.GPIO_pin,0)

void I2C_Delay(uint32_t us){
	uint32_t value = HAL_RCC_GetSysClockFreq();
	value/=1e6;
	value*=us;
	while(value-->0);
}

void SFT_I2C_Init(GPIO_TypeDef *SCL_GPIOx, uint16_t *SCL_GPIO_pin,GPIO_TypeDef *SDA_GPIOx, uint16_t *SDA_GPIO_pin,I2C_handler *hi2c,uint8_t params){
	if(hi2c!=NULL){
		hi2c->scl.GPIO_pin = SCL_GPIO_pin;
		hi2c->scl.GPIOx = SCL_GPIOx;
		hi2c->sda.GPIO_pin = SDA_GPIO_pin;
		hi2c->sda.GPIOx = SDA_GPIOx;
	}
	setLineDirection(&(hi2c->scl),I2C_OUTPUT|((params)&(I2C_PULLUP)));
	setLineDirection(&(hi2c->sda),I2C_OUTPUT|((params)&(I2C_PULLUP)));
}

void setLineDirection(struct sPinPack *line,uint8_t params){
	// 1 - output
	// 0 - input
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = line->GPIO_pin;
	GPIO_InitStruct.Mode = (params & I2C_INPUT)? GPIO_MODE_INPUT : GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = (params & I2C_PULLUP)? GPIO_PULLUP : GPIO_NOPULL;
	HAL_GPIO_Init(line->GPIOx, &GPIO_InitStruct);
	line->params = params;
}

void startCondition(struct Shandler *si2c){
	HAL_GPIO_WritePin(si2c->sda.GPIOx,si2c->sda.GPIO_pin,1);
	HAL_GPIO_WritePin(si2c->scl.GPIOx,si2c->scl.GPIO_pin,1);
	I2C_Delay(I2C_DELAY_US);
	HAL_GPIO_WritePin(si2c->sda.GPIOx,si2c->sda.GPIO_pin,0);
	I2C_Delay(I2C_DELAY_US);
	HAL_GPIO_WritePin(si2c->scl.GPIOx,si2c->scl.GPIO_pin,0);
	I2C_Delay(I2C_DELAY_US);
	HAL_GPIO_WritePin(si2c->sda.GPIOx,si2c->sda.GPIO_pin,1);
}

void stopCondition(struct Shandler *si2c){
	HAL_GPIO_WritePin(si2c->sda.GPIOx,si2c->sda.GPIO_pin,0);
	I2C_Delay(I2C_DELAY_US);
	HAL_GPIO_WritePin(si2c->scl.GPIOx,si2c->scl.GPIO_pin,1);
	I2C_Delay(I2C_DELAY_US);
	HAL_GPIO_WritePin(si2c->sda.GPIOx,si2c->sda.GPIO_pin,1);

}

void sendByte(struct Shandler *si2c,uint8_t byte){
	int i;
	for(i=7;i>=0;i--){
		HAL_GPIO_WritePin(si2c->sda.GPIOx,si2c->sda.GPIO_pin,byte&(1<<i));
		I2C_Delay(I2C_DELAY_US);
		HAL_GPIO_WritePin(si2c->scl.GPIOx,si2c->scl.GPIO_pin,1);
		I2C_Delay(I2C_DELAY_US);
		HAL_GPIO_WritePin(si2c->scl.GPIOx,si2c->scl.GPIO_pin,0);
	}
	HAL_GPIO_WritePin(si2c->sda.GPIOx,si2c->sda.GPIO_pin,1);
	I2C_Delay(I2C_DELAY_US);

}

uint8_t recvByte(struct Shandler *si2c,uint8_t lastByte){
	int i;
	uint8_t byte = 0;
	setLineDirection(&si2c->sda,I2C_INPUT|I2C_PULLUP);
	for(i=7;i>=0;i--){
		CLOCK_UP;
		I2C_Delay(I2C_DELAY_US);
		byte|=(HAL_GPIO_ReadPin(si2c->sda.GPIOx,si2c->sda.GPIO_pin))<<i;
		CLOCK_DOWN;
		I2C_Delay(I2C_DELAY_US);
	}
	setLineDirection(&si2c->sda,I2C_OUTPUT|I2C_PULLUP);
	if(!lastByte)
		HAL_GPIO_WritePin(si2c->sda.GPIOx,si2c->sda.GPIO_pin,ACK);
	CLOCK_UP;
	I2C_Delay(I2C_DELAY_US);
	CLOCK_DOWN;
	HAL_GPIO_WritePin(si2c->sda.GPIOx,si2c->sda.GPIO_pin,1);
	I2C_Delay(I2C_DELAY_US);
	return byte;
}

uint8_t readReceipt(struct Shandler *si2c){
	uint8_t result;
	HAL_GPIO_WritePin(si2c->scl.GPIOx,si2c->scl.GPIO_pin,1);
	setLineDirection(&si2c->sda,I2C_INPUT|I2C_PULLUP);
	I2C_Delay(I2C_DELAY_US);
	result = HAL_GPIO_ReadPin(si2c->sda.GPIOx,si2c->sda.GPIO_pin);
	setLineDirection(&si2c->sda,I2C_OUTPUT|I2C_PULLUP);
	HAL_GPIO_WritePin(si2c->scl.GPIOx,si2c->scl.GPIO_pin,0);
	HAL_GPIO_WritePin(si2c->sda.GPIOx,si2c->sda.GPIO_pin,1);
	I2C_Delay(I2C_DELAY_US);
	return result;
}

void SFT_I2C_Master_Transmit(struct Shandler *si2c, uint8_t DevAddress, uint8_t *pData,uint16_t Size, uint8_t timeout){
	uint16_t i = -1;
	startCondition(si2c);
	// address
	sendByte(si2c,DevAddress);
	// data
	while((readReceipt(si2c) == ACK) && ++i<Size)
		sendByte(si2c,pData[i]);
	stopCondition(si2c);
}

void SFT_I2C_Master_Receive (struct Shandler *si2c, uint8_t DevAddress, uint8_t *pData,uint16_t Size, uint8_t timeout){
	uint16_t i = -1;
	startCondition(si2c);
	// address
	sendByte(si2c,DevAddress|1);
	readReceipt(si2c);
	// data
	while(++i<Size)
		pData[i] = recvByte(si2c,(i+1==Size));
	stopCondition(si2c);
}
