/*
 * text.h
 *
 *  Created on: 10 сент. 2021 г.
 *      Author: nms
 */
#pragma once
#ifndef INC_TEXT_H_
#define INC_TEXT_H_


#define TST_PIO  GPIOA,GPIO_PIN_2
#define EXT_RST_PIN  GPIOB,GPIO_PIN_4
#define EXT_ALT_BOOT GPIOB,GPIO_PIN_5
#define EXT_PGOOD_PIN GPIOA,GPIO_PIN_15




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




#define WELCOME_SCREEN "INMYS MS-uQ7-BKLT\r\nHW ver.: "HW_VERSION"\r\nFW ver.: "FW_VERSION
#define HW_VERSION "1.0"
#define FW_VERSION "0.5"


extern const char *CS_STASTUS_LABELS[];
extern const char** menu[];
extern const char** text[];
extern const char* ASMNOP;


#endif /* INC_TEXT_H_ */
