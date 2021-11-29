/*
 * text.c
 *
 *  Created on: 10 сент. 2021 г.
 *      Author: nms
 */
#include "text.h"
const char *CS_STASTUS_LABELS[] = {"CONFIRMED","UPDATED","BAD","RESERVED"};


const char* menu[] = {"Select menu item","1) Boot","2) Update flash 1","3) Update flash 2","4) Toggle main flash",
		"5) Toggle boot flash", "6) Toggle watchdog","7) Set FW status to: CONFIRMED","8) Set FW status to: UPDATED","9) Set FW status to: BAD"};
const char* text[] = {"Choose CS (1/2):","Wrong value","Choose State (1-3):","You can't write main flash. Change Flash first."};

const char* SWT[] = {"ON","OFF",">>"};
