/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#pragma pack(0)

#define TST_PIO  GPIOA,GPIO_PIN_2
#define EXT_RST_PIN  GPIOB,GPIO_PIN_4
#define EXT_ALT_BOOT GPIOB,GPIO_PIN_5
#define EXT_PGOOD_PIN GPIOA,GPIO_PIN_15


#define WELCOME_SCREEN "\r\nINMYS Ltd. MS-uQ7-BKLT\r\nHW ver.: "HW_VERSION"\r\nFW ver.: "FW_VERSION"\r\n"
#define HW_VERSION "1.0"
#define FW_VERSION "0.5"

#define STR(a) #a


#define BUF_LEN 40
#define GPIO_EXPANDER_ADDR 0x20<<1

#define XMODEM_TIME_1SEC (100)

#define XMODEM_STATE_INIT (0)
#define XMODEM_STATE_S0 (1)
#define XMODEM_STATE_S1 (2)
#define XMODEM_STATE_S2 (3)
#define XMODEM_STATE_S3 (4)


#define UART_BUF_SIZE 32
#define RX_BUF_SIZE (256)

struct SSysCntrl{
	uint8_t power_stage;
	uint8_t PowerTimer;
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


	union{
		uint8_t TempInfo;
		struct{
			uint8_t BootFlash:1;
			uint8_t BootAttempt:2;
		};
	};

	union {
		struct{
		uint8_t SavedConfigH;
		uint8_t SavedConfigL;
		};
		struct{
			// boot settings
			uint8_t MainFlash:1; // 0/1
			uint8_t cs0:2;
			uint8_t cs1:2;
			uint8_t Watchdog:1; //  0/1
			uint8_t PowerState:1; // 0 - off by key / 1 - on
			uint8_t Magic:5; // 10110 - OK
			};
		};

	union{
		uint8_t IOStatus;
		struct{
			uint8_t pgin:1;
			uint8_t pwrbtn:1;
			uint8_t rstbtn:1;
			uint8_t stmbootsel:1;


		};
	};
}SysCntrl;


struct{
	uint8_t idx;
	uint8_t buf[UART_BUF_SIZE];
	uint8_t buf1[32];
	// cmdStage:
	// 0 print menu
	// 1 wait user input
	// 3 print prompt
	// 4 wait confirm prompt
	// 5 execute
	// 6 wait user result
	uint16_t cmdStage;
	uint8_t cmd_flag;
	uint8_t result;
	uint8_t cmdCode;
	uint8_t args[3];
} console;


enum CS_STASTUS{
	CONFIRMED = 0b00,
	UPDATED = 0b01,
	BAD = 0b10,
	RESERVED = 0b11
};


void EnableSPI();
void DisableSPI();


/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
