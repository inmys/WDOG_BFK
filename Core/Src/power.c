/*
 * power.c
 *
 *  Created on: Aug 5, 2021
 *      Author: vovav
 */
#include "structs.h"
#include "power.h"
#include "memory.h"
#include "text.h"

int BootMenu();

void PowerSM() {
	char buf[32] = {0};
	if(SysCntrl.PowerTimer>0)
		SysCntrl.PowerTimer--;
	else
		// 1 powerTick = 10ms
		switch(SysCntrl.power_stage) {
		// State: CPU is turned off & power is off
		//CPU is turned off & is not planing to going on
		case 100:
			//ClrI2C_Mask(ENA_LV_DCDC|ENA_HV_DCDC|TRST_N|EJ_TRST_N|RESETN,|CPU_RESET);
			ClrI2C_Mask(0b11111111);
			//UART_putstrln("IN STAGE 100");

			//AUTOBOOT HERE!!!!
			if(BootMenu() || SysCntrl.pwrbtn || SysCntrl.PowerState)
			//if(BootMenu() || SysCntrl.pwrbtn)
				SysCntrl.power_stage = 0;
			break;
		case 0:
			 if(SysCntrl.FWStatus == CONFIRMED || SysCntrl.FWStatus == BAD)
				 SysCntrl.BootFlash = SysCntrl.MainFlash;
			 else{
				 if(SysCntrl.BootAttempt>0){
					 SysCntrl.BootAttempt--;
					 SysCntrl.BootFlash = ~SysCntrl.MainFlash;
				 }
				 else
				 {
					 SysCntrl.FWStatus = BAD;
					 SysCntrl.BootFlash = SysCntrl.MainFlash;
				 }
				 //writeConfig();
			 }
			 SysCntrl.power_stage = 9;
		break;
		case 9:
			 if(SysCntrl.BootFlash){
				  SetI2C_Mask(FLASH_EN_1);
				  ClrI2C_Mask(FLASH_EN_0);
			  }
			  else{
				  SetI2C_Mask(FLASH_EN_0);
				  ClrI2C_Mask(FLASH_EN_1);
			  }
			if(SysCntrl.pgin){
				SysCntrl.power_stage = 10;
				SysCntrl.PowerTimer = 2;
			}
		break;
		// State: CPU is turned off & power is turning on STAGE 1
		case 10:
			SetI2C_Mask(TRST_N|EJ_TRST_N);
			ClrI2C_Mask(RESET_N|CPU_RST_N);
			SysCntrl.PowerTimer  = 2;
			SysCntrl.power_stage = 20;
		break;
		// State: CPU is turned off & power is turning on STAGE 2
		case 20:
			SetI2C_Mask(ENA_LV_DCDC);
			SysCntrl.PowerTimer  = 20;
			SysCntrl.power_stage = 30;
		break;
		// State: CPU is turned off & power is turning on STAGE 3
		case 30:
			SetI2C_Mask(ENA_HV_DCDC);
			// SUS_S3# set
			SysCntrl.PowerTimer  = 10;
			SysCntrl.power_stage = 40;
		break;
		// State: CPU is turned off & power is turning on STAGE 4
		case 40:
			SetI2C_Mask(CPU_RST_N);
			ClrI2C_Mask(TRST_N|EJ_TRST_N|RESET_N);
			SysCntrl.PowerTimer  = 100;
			SysCntrl.power_stage = 51;
			//WATHCDOG HARD OFF HERE!!!
			SysCntrl.Watchdog = 0;
			SysCntrl.WatchdogTimer = 0;
		break;
		// State: CPU is on & power is on NORMAL STATE
		case 51:
			SetI2C_Mask(TRST_N|EJ_TRST_N|RESET_N);
			ClrI2C_Mask(CPU_RST_N);
			SysCntrl.PowerTimer  = 100;
			if(SysCntrl.Watchdog && (SysCntrl.WatchdogTimer > 200*100)  ){ // 500 = 5 seconds
				//UART_putstrln("Attention: 90 sec no response. CPU restarted");
				SysCntrl.power_stage = 41;
				SysCntrl.WatchdogTimer = 0;
			}
			if(SysCntrl.rstbtn){
				SysCntrl.PowerTimer  = 40;
				SysCntrl.power_stage = 41;
			}
			if(SysCntrl.pwrbtn)
				SysCntrl.power_stage = 11;
		break;
		// State: CPU is on & requested soft reset
		case 41:
			SysCntrl.PowerTimer  = 500;
			SysCntrl.power_stage = 21;
		break;
		// initiating hard reset
		case 21:
			SetI2C_Mask(CPU_RST_N);
			ClrI2C_Mask(TRST_N|EJ_TRST_N|RESET_N);
			SysCntrl.PowerTimer  = 1;
			SysCntrl.power_stage = 51;
		break;
		case 11:
			SysCntrl.PowerTimer = 500;
			SysCntrl.power_stage = 100;
		}
}

// 0b01101111
// 0b01101111
// PGIN  power good INVERTED on PCB and additionaly inverted here
// RSTBTN reset btm from motherboard    (x16)
// PWRBTN power button from motherboard (x15)
// ALTBOOT watchdog                     (x14)
// STMBOOTSEL ???                       (x19)

uint8_t debouncer(GPIO_TypeDef * GPIOx, uint16_t GPIO_Pin){
	uint8_t pinState;
	uint8_t prevState;
	uint8_t i,swtch = 1;
	char buf[25] = {0};
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

}

void checkPowerLevels(uint8_t output){
	char buf[10] = {0};
	uint8_t pinState;


	pinState = HAL_GPIO_ReadPin(PGIN_PIN);

	SysCntrl.pgin = (pinState)?0:1;
	if(output){
		sprintf(buf,"PGIN: %d",SysCntrl.pgin);
		UART_putstrln(1,buf);
	}

	pinState = HAL_GPIO_ReadPin(PWRBTN_PIN);
	if((pinState&0b00000001)!=SysCntrl.pwrbtn){
		pinState = debouncer(PWRBTN_PIN);
		if(pinState&0b10000000)
			SysCntrl.pwrbtn = pinState&0b00000001;
	}

	if(output){
		sprintf(buf,"PWRBTN: %d",SysCntrl.pwrbtn);
		UART_putstrln(1,buf);
	}

	pinState = HAL_GPIO_ReadPin(RSTBTN_PIN);
	if((pinState&0b00000001)!=SysCntrl.rstbtn){
		pinState = debouncer(RSTBTN_PIN);
		if(pinState&0b10000000){
			SysCntrl.rstbtn = pinState&0b00000001;
		}
	}

	if(output){
		sprintf(buf,"RSTBTN: %d",SysCntrl.rstbtn);
		UART_putstrln(1,buf);
	}

	pinState = HAL_GPIO_ReadPin(STMBOOTSEL_PIN);
	if((pinState&0b00000001)!=SysCntrl.stmbootsel){
		pinState = debouncer(STMBOOTSEL_PIN);
		if(pinState&0b10000000)
			SysCntrl.stmbootsel = pinState&0b00000001;
	}

	if(output){
		sprintf(buf,"STMBOOTSEL: %d",SysCntrl.stmbootsel);
		UART_putstrln(1,buf);
	}
}



int BootMenu(){
	uint8_t i,result = 0;
	clearUartConsole();

	UART_putstrln(1,WELCOME_SCREEN);
	memoryMenu();
	UART_putstrln(0,"CPU FW status:");
	UART_putstrln(1,CS_STASTUS_LABELS[SysCntrl.FWStatus]);
	for(i=0;i<10;i++)
		UART_putstrln(1,menu[i]);
	UART_putstrln(0,">>");

	refreshConsoleBuffer();
	while(!console.cmd_flag)
		userInput(0);
	uint8_t cmd = atoi(console.buf)-1;
	switch(cmd){
	case 0:
		UART_putstrln(1,"Booting...");
		result = 1;
		break;
	case 1:
		// Update 1st flash
		if(SysCntrl.MainFlash == 0){
			UART_putstrln(1,text[3]);
			// ERROR
		}
		else{
			SysCntrl.active_cs = 0;
			Xmodem_Init();
		}
	break;
	case 2:
		// Update 1st flash
		if(SysCntrl.MainFlash == 1){
			UART_putstrln(1,text[3]);
			// ERROR
		}
		else{
			SysCntrl.active_cs = 1;
			Xmodem_Init();
		}
	break;
	case 3:
		// Toggle main flash
		SysCntrl.MainFlash = ~SysCntrl.MainFlash;
		writeConfig();
	case 4:
		// Toggle boot flash
		SysCntrl.BootFlash = ~SysCntrl.BootFlash;
	break;
	case 5:
		// Toggle watchdog
		SysCntrl.Watchdog = ~SysCntrl.Watchdog;
	break;
	case 6:
		// Set FW status to CONFIRMED
		SysCntrl.FWStatus = CONFIRMED;
	break;
	case 7:
		// Set FW status to UPDATED
		SysCntrl.FWStatus = UPDATED;
	break;
	case 8:
		// Set FW status to BAD
		SysCntrl.FWStatus = BAD;
	break;

	}
	refreshConsoleBuffer();
	return result;
}


