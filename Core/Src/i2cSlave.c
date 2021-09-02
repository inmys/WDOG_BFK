/*
 * i2cSlave.c
 *
 *  Created on: 25 авг. 2021 г.
 *      Author: vovav
 */

#include "i2cSlave.h"

void initReg(){
	i2cReg.x00 = 0;
	i2cReg.x01 = 0b00011000;
	i2cReg.x02 = 0b01000000;
	i2cReg.x03 = 0b00000000;
}

void updateReg(){
	;//i2cReg.
}
