/*
 * power.h
 *
 *  Created on: Aug 5, 2021
 *      Author: vovav
 */

#ifndef INC_POWER_H_
#define INC_POWER_H_

void PowerSM();
void checkPowerLevels(uint8_t output);


// POWER SUBSYSTEM SIGNALS
#define ENA_LV_DCDC  (1<<0)
#define ENA_HV_DCDC  (1<<1)
#define RESET_N      (1<<2) // 1V8_RESET_N
#define FLASH_EN_0   (1<<3)
#define FLASH_EN_1   (1<<4)
#define TRST_N       (1<<5)
#define EJ_TRST_N    (1<<6)
#define CPU_RST_N    (1<<7) // 1V8_CPU_RESET

#define PGIN_PIN GPIOA, GPIO_PIN_15
#define PWRBTN_PIN GPIOF, GPIO_PIN_1
#define RSTBTN_PIN GPIOB, GPIO_PIN_4
#define ALTBOOT_PIN  GPIOB, GPIO_PIN_5
#define STMBOOTSEL_PIN GPIOB, GPIO_PIN_8
#define CPU_INT GPIOA, GPIO_PIN_3


#define PGIN       (1<<0)
#define PWRBTN     (1<<1)
#define RSTBTN     (1<<2)
#define ALTBOOT    (1<<3)
#define STMBOOTSEL (1<<4)

#define TRST_N       (1<<5)
#define EJ_TRST_N    (1<<6)
#define CPU_RST_N    (1<<7)

#endif /* INC_POWER_H_ */
