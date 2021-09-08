#ifndef __MEMORY_H
#define __MEMORY_H

// STM32F048

#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_flash.h"

#define SPI_CS_0 GPIOB,GPIO_PIN_0
#define SPI_CS_1 GPIOB,GPIO_PIN_1



struct memoryReport{
	uint8_t ManufacturerID;
	uint8_t MemoryType;
	uint8_t MemoryCapacity;
	uint8_t UniqID[2];
};


typedef struct memoryReport memID;

uint8_t SPI_ReadStatus();
void Flash_WriteEnable();
void SPI_PageRead(uint32_t spi_address, uint8_t *data, uint32_t len);
void Flash_PageWrite();
void SPI_EraseAddr(uint32_t);
void SPI_Reset(uint8_t);
uint8_t SPI_ReadID(uint8_t,struct memoryReport*);
void Xmodem_SPI();
void Xmodem_Init();
void EnableSPI();
void DisableSPI();
void FlashDump(uint8_t cs);

void memoryMenu();

#define CONFIG_ADDR_IN_FLASH 0x08007C04
void writeConfig();
void readConfig();

#endif
