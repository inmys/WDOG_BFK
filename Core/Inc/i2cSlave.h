/*
 * i2cSlave.h
 *
 *  Created on: 25 авг. 2021 г.
 *      Author: vovav
 */


typedef unsigned char uint8_t ;
#ifndef INC_I2CSLAVE_H_
#define INC_I2CSLAVE_H_
#include "Structs.h"

void i2cSM();


#define I2C_PWRBTN SysCntrl.pwrbtn<<0
#define I2C_RSTBTN SysCntrl.rstbtn<<1
#define I2C_BOOTPIN_POS 2
#define I2C_BOOTPIN SysCntrl.stmbootsel<<I2C_BOOTPIN_POS
#define I2C_WDOG_POS 3
#define I2C_WDOG SysCntrl.Watchdog<<I2C_WDOG_POS
#define I2C_INTEN_POS 4
#define I2C_INTEN SysCntrl.intEn<<I2C_INTEN_POS
#define I2C_DORESET (SysCntrl.power_stage==41)<<5
#define I2C_DOPWROFF (SysCntrl.power_stage==11)<<6
#define I2C_BOOTLDR_POS 7
#define I2C_BOOTLDR SysCntrl.bootloaderMode<<I2C_BOOTLDR_POS



#define I2C_RREG0 I2C_BOOTLDR|I2C_DOPWROFF|I2C_DORESET|I2C_INTEN|I2C_WDOG|I2C_BOOTPIN|I2C_RSTBTN|I2C_PWRBTN
#define I2C_RREG1 SysCntrl.MainFlash<<2|SysCntrl.FWStatus
#define I2C_RREG2 FW_VERMJR<<4|FW_VERMNR


#endif /* INC_I2CSLAVE_H_ */
