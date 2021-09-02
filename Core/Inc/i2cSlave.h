/*
 * i2cSlave.h
 *
 *  Created on: 25 авг. 2021 г.
 *      Author: vovav
 */


typedef unsigned char uint8_t ;
#ifndef INC_I2CSLAVE_H_
#define INC_I2CSLAVE_H_

struct Sregs{
	union{
		uint8_t x00;
		struct{
			uint8_t ResetBtn:1;
			uint8_t PWRbtn:1;
			uint8_t Reserved0:6;
		};
	};
	union{
		uint8_t x01;
		struct{
			uint8_t BootflashN:1;
			uint8_t doPWRoff:1;
			uint8_t doReset:1;
			uint8_t intEn:1;
			uint8_t WDT:1;
			uint8_t Reserved1:1;
			uint8_t bootldr:1;
			uint8_t Reserved2:1;
		};
	};

	union{
		uint8_t x02;
		struct{
			uint8_t FWst1:1;
			uint8_t FWst0:1;
			uint8_t bootpin:1;
			uint8_t FlashEn:1;
			uint8_t MainFlashN:1;
			uint8_t Reserved3:3;
		};
	};

	union{
			uint8_t x03;
			struct{
				uint8_t FWmjr3:1;
				uint8_t FWmjr2:1;
				uint8_t FWmjr1:1;
				uint8_t FWmjr0:1;
				uint8_t FWmnr3:1;
				uint8_t FWmnr2:1;
				uint8_t FWmnr1:1;
				uint8_t FWmnr0:1;
			};
		};

} i2cReg;

void initReg();

#endif /* INC_I2CSLAVE_H_ */
