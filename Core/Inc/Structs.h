/*
 * Structs.h
 *
 *  Created on: 10 сент. 2021 г.
 *      Author: nms
 */

#ifndef SRC_STRUCTS_H_
#define SRC_STRUCTS_H_
#include<stdint.h>
#include"text.h"

struct SSysCntrl{
	uint8_t power_stage;
	uint16_t PowerTimer;
	// power subsystem signal values (see POWER SUBSYSTEM SIGNALS in power header)
	uint8_t i2c_bt[2];
	uint8_t active_cs;
	uint8_t spi_buf_tx[16];
	uint8_t spi_buf_rx[16];

	uint8_t uart_rx_buf[RX_BUF_SIZE];
	uint16_t rx_head;
	uint16_t rx_tail;


	uint8_t SPI_page[256];
	uint8_t SPI_rxbuf[140];
	uint32_t SPI_page_idx;
	uint32_t SPI_address;
	uint32_t t_idx;
	uint32_t X_idx;
	uint32_t bt_count;
	uint8_t XmodemState;
	uint8_t XmodemMode:1;
	uint32_t TimerCnt;
	uint8_t TryCounter;


	uint8_t TimerTick:1;
	uint16_t MS_counter;
	// Memory configuration

	union {
		struct{
		uint8_t SavedConfigH;
		uint8_t SavedConfigL;
		};
		struct{
			// boot settings
			uint8_t MainFlash:1; // 0/1
			uint8_t FWStatus:2;
			uint8_t BootAttempt:2;
			uint8_t PowerState:1; // 0 - off by key / 1 - on
			uint8_t Magic:5; // 10110 - OK
			};
	};

	union{
		uint16_t IOStatus;
		struct{
			uint8_t pgin:1;
			uint8_t pwrbtn:1;
			uint8_t rstbtn:1;
			uint8_t stmbootsel:1;
			uint8_t BootFlash:1;
			uint8_t Watchdog:1; //  0/1
			uint8_t intEn:1;
			uint8_t bootloaderMode:1; // 0/1
			};
		};
		uint16_t WatchdogTimer;
}SysCntrl;


struct SConsole{
	uint8_t idx;
	uint8_t buf[UART_BUF_SIZE];
	uint8_t cmd_flag;
	uint8_t result;
	uint8_t cmdCode;
	uint8_t args[3];
	uint8_t timer;
} console;


enum CS_STASTUS{
	CONFIRMED = 0b00,
	UPDATED = 0b01,
	BAD = 0b10,
	RESERVED = 0b11
};

struct Si2c{
	union{
		uint8_t config;
		struct{
			// States
			// 1 - preparing data to be sent
			// 2 - sending

			// 3 - preparing data to be read
			// 4 - reading
			uint8_t state:3;
			uint8_t bufIdx:2;
			uint8_t INT;
		};
	};
	uint8_t address;
	uint8_t registers[3];
}hi2c;

#endif /* SRC_STRUCTS_H_ */
