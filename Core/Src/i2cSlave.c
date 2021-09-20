/*
 * i2cSlave.c
 *
 *  Created on: 25 авг. 2021 г.
 *      Author: vovav
 */

#include "i2cSlave.h"

#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_i2c.h"

uint16_t readWord();
void writeWord(uint16_t);

uint8_t I2C_Slave_Registers[3];



void i2cSM(){
	 //char *MazzyStar = "I wanna hold the hand inside you  I wanna take the breath that's true  I look to you and I see nothing  I look to you to see the truth  You live your life, you go in shadows  You'll come apart and you'll go black  Some kind of night into your darkness  Colors your eyes with what's not there";
	uint8_t byte;
	if ((I2C1->ISR & I2C_ISR_ADDR) == I2C_ISR_ADDR){
	   I2C1->ICR |= I2C_ICR_ADDRCF;
	   if ((I2C1->ISR & I2C_ISR_DIR) == I2C_ISR_DIR){
		   I2C1->CR1 |= I2C_CR1_TXIE;
		   hi2c.state=1;
		   hi2c.address = readWord();
	   }
	   else
		   if(hi2c.state == 1)
			   hi2c.state = 3;
	}
	else{
		switch(hi2c.state){
		case 1:
			UART_putstrln("STM->Baikal");
			hi2c.state = 2;
		break;
		case 2:
			
			switch(hi2c.address){
				// Input Port Registers
				case 0:
					byte = I2C_RREG0;
				break;
				case 1:
					byte = I2C_RREG1;
				break;
				case 2:
					byte = I2C_RREG2;
				break;
				// Configuration Registers
				case 12:
				case 13:
				case 14:
					byte = I2C_Slave_Registers[hi2c.address-12];
				break;
				
				default:
					byte = 0xad;
				break;
			}
			writeWord(byte);
			//writeWord(MazzyStar[hi2c.address]);
			hi2c.state = 5;
			break;

		case 3:
			UART_putstrln("Baikal->STM");
			hi2c.state = 4;
			break;
		case 4:
			byte = readWord();
			switch(hi2c.address){
				// Input Port Registers
				case 0:
				// Configuration Registers
				case 4:
					SysCntrl.bootloaderMode = (byte&(1<<I2C_BOOTLDR_POS))?1:0;
					SysCntrl.stmbootsel = byte&(1<<I2C_BOOTPIN_POS))?1:0;
					SysCntrl.Watchdog = (byte&(1<<I2C_WDOG_POS))?1:0;
					SysCntrl.intEn = (byte&(1<<I2C_INTEN_POS))?1:0;
				break;
				
				default:
					byte = 0xad;
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

uint16_t readWord(){
	uint16_t word = 0;
  if ((I2C1->ISR & I2C_ISR_RXNE) == I2C_ISR_RXNE)
	  word = I2C1->RXDR;
  return word;
}
void writeWord(uint16_t word){
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
