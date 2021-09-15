/*
 * i2cSlave.h
 *
 *  Created on: 25 авг. 2021 г.
 *      Author: vovav
 */


typedef unsigned char uint8_t ;
#ifndef INC_I2CSLAVE_H_
#define INC_I2CSLAVE_H_

extern short int portConf;

#define COMPOSE_OUTPUT_REG 0xFD

void initReg();

#endif /* INC_I2CSLAVE_H_ */
