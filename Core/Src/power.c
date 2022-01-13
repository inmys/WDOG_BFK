/*
 * power.c
 *
 *  Created on: Aug 5, 2021
 *      Author: vovav
 */
#include "main.h"
#include "structs.h"
#include "power.h"
#include "memory.h"
#include "text.h"
#include "stdio.h"
#include "stdlib.h"

int BootMenu();

void PowerSM() {
	if(SysCntrl.PowerTimer>0)
		SysCntrl.PowerTimer--;
	else
		// 1 powerTick = 10ms
		switch(SysCntrl.power_stage) {
		// State: CPU is turned off & power is off
		//CPU is turned off & is not planing to going on
		case 0:
			ClrI2C_Mask(0b11111111);

			if(BootMenu() || SysCntrl.pwrbtn || (SysCntrl.Autoboot && !console.SecondsToStart))
				SysCntrl.power_stage = 1;
			break;
		case 1:
			UART_putstrln(1,"Booting...");
			if(!SysCntrl.UserChangedBootLogic)
				if(SysCntrl.FWStatus == CONFIRMED || SysCntrl.FWStatus == BAD)
					SysCntrl.BootFlash = SysCntrl.MainFlash;
				else{
					if(SysCntrl.BootAttempt==0){
						SysCntrl.BootAttempt--;
						SysCntrl.BootFlash = ~SysCntrl.MainFlash;
						writeConfig();
					}
					else
					{
						SysCntrl.FWStatus = BAD;
						SysCntrl.BootFlash = SysCntrl.MainFlash;
						SysCntrl.BootAttempt = 3;
						writeConfig();
					}
				}
			 SysCntrl.power_stage = 2;
		break;
		case 2:
			 if(SysCntrl.BootFlash){
				  SetI2C_Mask(FLASH_EN_1);
				  ClrI2C_Mask(FLASH_EN_0);
			  }
			  else{
				  SetI2C_Mask(FLASH_EN_0);
				  ClrI2C_Mask(FLASH_EN_1);
			  }
			 SysCntrl.power_stage = 3;
		break;
		// State: CPU is turned off & power is turning on STAGE 1
		case 3:
			SetI2C_Mask(TRST_N|EJ_TRST_N);
			ClrI2C_Mask(RESET_N|CPU_RST_N);
			SysCntrl.PowerTimer  = 1;
			SysCntrl.power_stage = 4;
		break;
		// State: CPU is turned off & power is turning on STAGE 2
		case 4:
			SetI2C_Mask(ENA_LV_DCDC);
			SysCntrl.PowerTimer  = 1;
			SysCntrl.power_stage = 5;
		break;
		// State: CPU is turned off & power is turning on STAGE 3
		case 5:
			SetI2C_Mask(ENA_HV_DCDC);
			// SUS_S3# set
			SysCntrl.PowerTimer  = 5;
			SysCntrl.power_stage = 6;
		break;
		// State: CPU is turned off & power is turning on STAGE 4
		case 6:
			SetI2C_Mask(CPU_RST_N);
			ClrI2C_Mask(TRST_N|EJ_TRST_N|RESET_N);
			SysCntrl.PowerTimer  = 50;
			SysCntrl.power_stage = 7;
			SysCntrl.WatchdogTimer = 0;
			UART_putstrln(0,SWT[2]);
		break;
		// State: CPU is on & power is on NORMAL STATE
		case 7:
			SetI2C_Mask(TRST_N|EJ_TRST_N|RESET_N);
			ClrI2C_Mask(CPU_RST_N);
			SysCntrl.PowerTimer  = 100;
			if(SysCntrl.rstbtn || (SysCntrl.WatchdogConsole && SysCntrl.WatchdogBootAlt && (SysCntrl.WatchdogTimer > MAIN_TIME_SCALER*85))  ){ // boot 85s min waiting, after i2c response 1 min
				SysCntrl.power_stage = 9;
				UART_putstrln(1,"restarting CPU");
			}
			if(SysCntrl.pwrbtn){
				UART_putstrln(1,"turning off CPU");
				SysCntrl.power_stage = 8;
			}
		break;
		// State: CPU is on & requested soft shutdown
		case 8:
			SysCntrl.PowerTimer  = 50;
			SysCntrl.power_stage = 10;
		break;
		// initiating hard reset
		case 9:
			SetI2C_Mask(CPU_RST_N);
			ClrI2C_Mask(TRST_N|EJ_TRST_N|RESET_N);
			SysCntrl.PowerTimer  = 1;
			SysCntrl.power_stage = 7;
			SysCntrl.WatchdogTimer = 0;

		break;
		case 10:
			ClearConsoleBuffer();
			SysCntrl.PowerTimer = 25;
			SysCntrl.power_stage = 0;
		break;
		default:
			UART_putstrln(1,"Unknown powerstate");
		}
}

// 0b01101111
// 0b01101111
// PGIN  power good INVERTED on PCB and additionaly inverted here
// RSTBTN reset btm from motherboard    (x16)
// PWRBTN power button from motherboard (x15)
// ALTBOOT turn watchdog  on/off        (x14)
// STMBOOTSEL ???                       (x19)

uint8_t debouncer(GPIO_TypeDef * GPIOx, uint16_t GPIO_Pin){
	uint8_t pinState;
	uint8_t prevState;
	uint8_t i,swtch = 1;
	pinState = HAL_GPIO_ReadPin(GPIOx,GPIO_Pin);
	for(i=0;i<3 && swtch;i++){
		prevState = pinState;
		pinState = HAL_GPIO_ReadPin(GPIOx,GPIO_Pin);
		if(prevState!=pinState)
			swtch = 0;
		else
			HAL_Delay(2);
	}

	if(swtch)
		return 0b10000000|((pinState)?1:0);
	return ((pinState)?1:0);
}

void checkPowerLevels(){
	char buf[10] = {0};
	uint8_t pinState;

	pinState = HAL_GPIO_ReadPin(PWRBTN_PIN);
	if((pinState&0b00000001)!=SysCntrl.pwrbtn){
		pinState = debouncer(PWRBTN_PIN);
		if(pinState&0b10000000)
			SysCntrl.pwrbtn = pinState&0b00000001;
	}


	pinState = HAL_GPIO_ReadPin(RSTBTN_PIN);
	if((pinState&0b00000001)!=SysCntrl.rstbtn){
		pinState = debouncer(RSTBTN_PIN);
		if(pinState&0b10000000)
			SysCntrl.rstbtn = pinState&0b00000001;
	}

	pinState = HAL_GPIO_ReadPin(ALTBOOT_PIN);
	if(pinState!=SysCntrl.WatchdogBootAlt)
		SysCntrl.WatchdogBootAlt = pinState;


}

int BootMenu(){
	uint8_t i,result = 0;
	char buf[25];
	const char* menu[] = {"Select menu item","1) Boot","2) Update flash 1","3) Update flash 2","4) Toggle main flash",
			"5) Toggle boot flash", "6) Toggle watchdog","7) Set FW status to: CONFIRMED","8) Set FW status to: UPDATED","9) Set FW status to: BAD","10) Update MCU","Enter selected number and press <Enter>"};
	switch(console.bootMenuStage){
	case 0:
		clearUartConsole();
		ClearConsoleBuffer();
		UART_putstrln(1,WELCOME_SCREEN);
		memoryMenu();
		POST();
		UART_putstrln(0,"CPU FW status:");
		UART_putstrln(1,CS_STASTUS_LABELS[SysCntrl.FWStatus]);
		for(i=0;i<12;i++)
			UART_putstrln(1,menu[i]);
		if(SysCntrl.Autoboot){
			sprintf(buf,"Starting after %d sec",console.SecondsToStart);
			UART_putstrln(1,buf);
		}
		console.bootMenuStage = 1;
		console.BootTimeout = 50;
		break;
	case 1:
		userInput(0);
		if(console.cmd_flag)
			console.bootMenuStage = 2;
		break;
	case 2:
		console.cmd = atoi(console.buf);
		UART_putstrln(1,NULL);
		ClearConsoleBuffer();
		if(console.cmd>1){
			console.bootMenuStage = 3;
			UART_putstrln(1,menu[console.cmd]);
			UART_putstrln(0,text[4]);
			}
		else
			console.bootMenuStage = 5;
		break;

	case 3:
		userInput(1);
		if(console.cmd_flag)
			console.bootMenuStage = 4;
		break;
	case 4:
		if(console.buf[0] == 'Y' || console.buf[0] == 'y')
			console.bootMenuStage = 5;
		else
			console.bootMenuStage = 0;
		break;
	case 5:
		UART_putstrln(1,NULL);
		switch(console.cmd){
			case 1:
				ClearConsoleBuffer();
				result = 1;
				break;
			case 2:
				// Update 1st flash
				if(SysCntrl.MainFlash == 0){
					UART_putstrln(1,text[3]);
					ClearConsoleBuffer();
					console.bootMenuStage = 7;
				}
				else{
					SysCntrl.active_cs = 0;
					Xmodem_Init();
				}
			break;
			case 3:
				// Update 1st flash
				if(SysCntrl.MainFlash == 1){
					UART_putstrln(1,text[3]);
					ClearConsoleBuffer();
					console.bootMenuStage = 7;
				}
				else{
					SysCntrl.active_cs = 1;
					Xmodem_Init();
				}
			break;
			case 4:
				// Toggle main flash
				SysCntrl.MainFlash = ~SysCntrl.MainFlash;
				writeConfig();
			break;
			case 5:
				// Toggle boot flash
				SysCntrl.BootFlash = ~SysCntrl.BootFlash;
				SysCntrl.UserChangedBootLogic = 1;
			break;
			case 6:
				// Toggle watchdog
				SysCntrl.WatchdogConsole = ~SysCntrl.WatchdogConsole;
			break;
			case 7:
				// Set FW status to CONFIRMED
				SysCntrl.FWStatus = CONFIRMED;
				SysCntrl.BootAttempt = 3;
				writeConfig();
			break;
			case 8:
				// Set FW status to UPDATED
				SysCntrl.FWStatus = UPDATED;
				SysCntrl.BootAttempt = 3;
				writeConfig();
			break;
			case 9:
				// Set FW status to BAD
				SysCntrl.FWStatus = BAD;
				SysCntrl.BootAttempt = 3;
				writeConfig();
			break;
			case 10:
				DFUMode();
			break;
			}
		console.bootMenuStage = 6;
		break;
		case 6:
			SysCntrl.Autoboot = 0;
			console.bootMenuStage = 0;
			break;
		case 7:
			userInput(1);
			if(console.cmd_flag)
				console.bootMenuStage = 0;
			break;
	}

	if(SysCntrl.Autoboot){
		console.BootTimeout--;
		if(!console.BootTimeout){
			console.bootMenuStage = 0;
			console.SecondsToStart--;
		}
	}

	return result;
}


