/*
 * i2cSlave.c
 *
 *  Created on: 25 авг. 2021 г.
 *      Author: vovav
 */

#include "i2cSlave.h"
#include "power.h"
#include "memory.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_i2c.h"
#include "stdio.h"
uint32_t readWord();
void writeWord(uint8_t);
void clearHi2c();
uint8_t confReg0;

void checkChange();
// 1 попадаем
// байкал хочет записать
// stm32 будет записывать

// 2 попадаем
// байкал хочет прочитать
// stm32 будет писать

void i2cSM(){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	uint8_t byte;
	char buf[25];


	checkChange();


	if (((I2C1->ISR) & I2C_ISR_ADDR) == I2C_ISR_ADDR){
	   I2C1->ICR |= I2C_ICR_ADDRCF;
	   if ((I2C1->ISR & I2C_ISR_DIR) == I2C_ISR_DIR){
		   hi2c.state=1;
		   I2C1->CR1 |= I2C_CR1_TXIE;

	   }
	   else{
		   hi2c.state=3;
		   hi2c.address = readWord();
	   }
	}
	else{
		switch(hi2c.state){
		case 1:
			UART_putstrln(1,"i2c:STM->Baikal"); //i2cget
			hi2c.state = 2;
			HAL_GPIO_WritePin(CPU_INT,0);
			break;
		case 2:
			hi2c.state = 5;
			switch(hi2c.address){
				// Input Port Registers
				case 0:
					byte = 0xDA;
				break;
				case 1:
					byte = 0xBE;
				break;
				case 2:
					byte = 0xEF;
				break;
				// Configuration Registers
				case 12:
					byte = confReg0;
				case 13:
				case 14:
					byte = 0;
				break;
				default:
					byte = 0xad;
				break;
			}
			writeWord(byte);
			break;

		case 3:
			UART_putstrln(1,"i2c:Baikal->STM"); //i2cset
			hi2c.state = 4;
			break;
		case 4:
			byte = readWord();

			switch(hi2c.address){
				// Input Port Registers
				case 0:
				// Configuration Registers
				case 3:
					if((byte&(1<<I2C_WDOG_POS))?1:0)
						SysCntrl.WatchdogTimer = 10000;
					SysCntrl.intEn = 0;
					writeConfig();
				break;
				case 4:
				case 12:
					confReg0 = byte;
				case 13:
				case 14:
					break;
				default:
				break;
			}
			hi2c.state = 5;
			break;
		case 5:
			clearHi2c();
			hi2c.state = 0;
			break;
		}
	}
}

uint32_t readWord(){
	uint32_t word = 0;
	HAL_Delay(150);
	if (((I2C1->ISR) & I2C_ISR_RXNE)==I2C_ISR_RXNE)
		word = I2C1->RXDR;
	return word;
}
void writeWord(uint8_t word){
	if ((I2C1->ISR & I2C_ISR_TXIS) == I2C_ISR_TXIS){
		I2C1->CR1 &=~ I2C_CR1_TXIE;
		I2C1->TXDR = word;
	}
}


void clearHi2c(){
	hi2c.bufIdx = 0;
	hi2c.address = 0;

	I2C1->CR1 &= ~I2C_CR1_PE;
	asm("nop");
	asm("nop");
	I2C1->CR1 |= I2C_CR1_PE;
}

void checkChange(){
	uint8_t result = 0;
	result|=hi2c.registers[0]^I2C_RREG0;
	result|=hi2c.registers[1]^I2C_RREG1;
	result|=hi2c.registers[2]^I2C_RREG2;

	if(result>0){
		hi2c.registers[0] = I2C_RREG0;
		hi2c.registers[1] = I2C_RREG1;
		hi2c.registers[2] = I2C_RREG2;
		HAL_GPIO_WritePin(CPU_INT,1);
	}
}
