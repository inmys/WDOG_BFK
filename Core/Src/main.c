/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c.h"
#include "usbd_cdc_if.h"
#include "memory.h"
#include "POST.h"
#include "power.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */


I2C_handler si2c1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_ADC_Init(void);
/* USER CODE BEGIN PFP */

const char *CS_STASTUS_LABELS[] = {"CONFIRMED","UPDATED","BAD","RESERVED"};

void EnableSPI() {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	ClrI2C_Mask(FLASH_EN_0|FLASH_EN_1);
	MX_SPI1_Init();
  	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_3;
  	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  	GPIO_InitStruct.Pull = GPIO_NOPULL;
  	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	Set_CS(0);
	Set_CS(1);

}

void DisableSPI() {

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	HAL_SPI_DeInit(&hspi1);
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_3;
  	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  	GPIO_InitStruct.Pull = GPIO_NOPULL;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void SetI2C_Mask(uint8_t mask) {
	SysCntrl.i2c_bt[1] |= mask;
	SFT_I2C_Master_Transmit(&si2c1,GPIO_EXPANDER_ADDR, SysCntrl.i2c_bt,2,1);
}
void ClrI2C_Mask(uint8_t mask) {
	SysCntrl.i2c_bt[1] &= ~mask;
	SFT_I2C_Master_Transmit(&si2c1,GPIO_EXPANDER_ADDR, SysCntrl.i2c_bt,2,1);
}

void Set_CS(uint8_t cs) {
	if(cs == 0)
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
	if(cs == 1)
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);

}
void Clr_CS(uint8_t cs) {
	if(cs == 0)
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
	if(cs == 1)
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);

}


void Test_RxPacket(uint8_t *Buf,uint32_t Len) {
	uint32_t idx;
	idx = 0;
	while(Len!=0) {
		SysCntrl.uart_rx_buf[SysCntrl.rx_head++] = Buf[idx++];
		SysCntrl.rx_head &= RX_BUF_SIZE-1;
		Len--;
	}
}

void UART_putstr(char *str){
	while(CDC_Transmit_FS((uint8_t*)str,strlen(str)) != USBD_OK);
}


// ПРОВЕРИТЬ
void UART_putstrln(char *str){
	while(CDC_Transmit_FS((uint8_t*)str,strlen(str)) != USBD_OK);
	HAL_Delay(1);
	while(CDC_Transmit_FS((uint8_t*)"\r\n",3) != USBD_OK);
}


void UART_SendByte(uint8_t bt) {
	while(CDC_Transmit_FS(&bt,1) != USBD_OK);
}

int ReadUartNonBlock(uint8_t *buf,int size) {
	int cnt;
	cnt = 0;

	while((SysCntrl.rx_head != SysCntrl.rx_tail) && size) {
		buf[cnt++] = SysCntrl.uart_rx_buf[SysCntrl.rx_tail++];
		SysCntrl.rx_tail &= RX_BUF_SIZE-1;
		size--;
	}
	return cnt;
}

const char** menu[] = {"1) Boot","2) Update flash 0","3) Update flash 1","4) Toggle main flash",
		"5) Toggle boot flash", "6) Toggle watchdog","7) Edit flash status"};
const char** text[] = {"Choose CS (1/2):","Wrong value","Choose State (1-3):"};

void userInput(){
	uint8_t bt;
	do{
		console.result = ReadUartNonBlock(&bt, 1);
		if(console.result) {
			if(bt == 0x8){ //0x8 - asci backspace
				UART_SendByte(0x8);
				UART_SendByte(0x20); //0x20 - asci space
				UART_SendByte(0x8);

				console.buf[--console.idx] = 0;
			}
			else{
				UART_SendByte(bt);
				if(bt == '\r'){
					console.buf[console.idx++] = 0;
					console.cmd_flag = 1;
				}else{
					console.buf[console.idx++] = bt;
				}
			}
			if(console.idx >= UART_BUF_SIZE) console.idx = 0;
		}

	}while(console.result && (!console.cmd_flag));
}

void refreshConsoleS(){
	int i;
	for(i=0;i<UART_BUF_SIZE;i++)
		console.buf[i] = 0;
	console.cmd_flag = 0;
}


void UART_Con_Mash(){
	char buf[32] = {0};
	uint8_t i,j;
	if(SysCntrl.XmodemMode) {
		Xmodem_SPI();return;
	}
	switch(console.cmdStage){
	case 0:
		console.cmdStage=1;
		// temp way
		UART_putstrln("\r\n\r\n\r\n\r\n\r\n\r\n");
		UART_putstrln("--------");
		memoryMenu();
		UART_putstrln("--------");
		for(i=0;i<7;i++)
			UART_putstrln(menu[i]);
		UART_putstr(">>");
		refreshConsoleS();
	break;
	case 1:
		// choose menu
		userInput();
		if(console.cmd_flag){
			console.cmdStage=2;
		}
	break;
	case 2:
		// decode text command
		console.idx = 0;
		if(!strcmp(console.buf,"help")){
			UART_putstrln("ping!\n\r");
			return;
		}
		else
		if(!strcmp(console.buf,"restart")){
			SysCntrl.power_stage = 41;
			UART_putstrln("CPU restarted...");
			console.cmdStage = 6;
			return;
		}
		else
		if(!strcmp(console.buf,"poweroff")){
			SysCntrl.power_stage = 100;
			UART_putstrln("CPU turn off...");
			console.cmdStage = 6;
			return;
		}
		else
		if(!strcmp(console.buf,"xmodem")){
			UART_putstrln("Start XMODEM");
			Xmodem_Init();console.cmdStage = 0;
			return;
		}
		else
		if(!strcmp(console.buf,"dump1")){
			EnableSPI();
			FlashDump(1);
			DisableSPI();
			console.cmdStage = 6;
			return;
		}
		else
		if(!strcmp(console.buf,"dump0")){EnableSPI();FlashDump(0);DisableSPI();console.cmdStage = 6;return;}
		else
		if(!strcmp(console.buf,"post")){POST();console.cmdStage = 6;return;}
		else
		if(!strcmp(console.buf,"power")){checkPowerLevels(1); return;}
		else
		if(!strcmp(console.buf,"mm")){memoryMenu(1); return;}
		else
		if(!strcmp(console.buf,"pwrState")){
			sprintf(buf,"power stage:%d",SysCntrl.power_stage);
			UART_putstrln(buf);console.cmdStage = 7;return;}
		else
		{
			console.cmdCode = atoi(console.buf);
			if(console.cmdCode>0 && console.cmdCode<11)
				console.cmdStage = 3;
			else
				console.cmdStage = 0;
		}
	break;
	case 3:
		// check if user is okay
		UART_putstrln(menu[console.cmdCode-1]);
		UART_putstr("Are you sure?(y/n)");
		console.cmdStage = 4;
		refreshConsoleS();
	break;
	case 4:
		// check user prompt
		userInput();
		if(console.cmd_flag){
			if((!strcmp(console.buf,"Y")) || (!strcmp(console.buf,"y")))
				console.cmdStage = 5;
			else
				console.cmdStage = 0;

			console.cmd_flag = 0;
		}
	break;
	case 5:
		// decode commands
			switch(console.cmdCode){
			case 1:
				// Boot
				UART_putstrln("Booting...");
				SysCntrl.PowerState = 1;
				SysCntrl.power_stage = 0;
				console.cmdCode = 0;
				console.cmdStage = 6;
			break;
			case 2:
				// Update 1st
				Clr_CS(0);
				Set_CS(1);
				Xmodem_Init();
			break;
			case 3:
				// Update 2nd
				Clr_CS(1);
				Set_CS(0);
				Xmodem_Init();
			break;
			case 4:
				// Toggle main
				SysCntrl.MainFlash = ~SysCntrl.MainFlash;
				writeConfig();
				console.cmdStage = 6;
			break;
			case 5:
				// Toggle boot
				SysCntrl.BootFlash = ~SysCntrl.BootFlash;
				writeConfig();
				console.cmdStage = 6;
			break;
			case 6:
				// Toggle watchdog
				SysCntrl.Watchdog = ~SysCntrl.Watchdog;
				console.cmdStage = 6;
			break;
			case 7:
				// Set Flash status
				console.cmdStage=700;
				;
			break;

			}
	break;
	case 6:
		UART_putstr("\r\nPress any key to continue");
		console.cmd_flag = 0;
		console.cmdStage = 7;
		break;
	case 7:
		userInput();
		if(console.cmd_flag){
				console.cmdStage = 0;
		}
		break;
	case 700:
		UART_putstrln(text[0]);
		console.cmdStage = 701;
		refreshConsoleS();
		break;
	case 701:
		// choose CS 0/1
		userInput();
			UART_putstrln("HERE HERE");
		if(console.cmd_flag){
			console.args[0] = atoi(console.buf);
			if(console.args[0] != 1 && console.args[0] != 2)
				console.cmdStage = 799;
			else
				console.cmdStage = 702;
			UART_putstrln(text[2]);
		}
		break;
	case 702:
		// choose state
		userInput();
		if(console.cmd_flag){
			console.args[0] = atoi(console.buf);
			if(console.args[0] != 1 && console.args[0] != 2)
				console.cmdStage = 799;
			else{
				console.cmdStage = 703;
				UART_pustrln(text[2]);
			}
		}
	case 703:
		userInput();
		if(console.cmd_flag){
			console.args[1] = atoi(console.buf);
			if(console.args[1] != 1 && console.args[1] != 2 && console.args[1] != 3)
				console.cmdStage = 799;
			else{
				console.cmdStage = 6;
				if(console.args[0]==1)
					SysCntrl.cs0 = console.args[1]-1;
				else
					SysCntrl.cs1 = console.args[1]-1;
			}
		}

		break;
	case 799:
		UART_putstrln(text[1]);
		console.cmdStage = 6;
		break;
	}

	}



extern USBD_HandleTypeDef hUsbDeviceFS;


uint8_t ByteToHEX(uint8_t bt){

if(bt<10)
	return bt+'0';
if(bt<=0xf)
	return bt+0x37;
return 'X';

}


void printBin(uint8_t bin){
	char buf[23] = {0};
	sprintf(buf,"0b%d%d%d%d%d%d%d%d",(bin&(1<<7))?1:0,(bin&(1<<6))?1:0,(bin&(1<<5))?1:0,(bin&(1<<4))?1:0,(bin&(1<<3))?1:0,(bin&(1<<2))?1:0,(bin&(1<<1))?1:0,(bin&(1<<0))?1:0);
	UART_putstrln(buf);
}

uint8_t wlcm = 1;

void main2(){
	//uint8_t i;
	//uint8_t buf[10] = {0};
	checkPowerLevels(0);
	switch(hUsbDeviceFS.dev_state){
		case USBD_STATE_CONFIGURED:
			if(wlcm){
			  UART_putstrln(WELCOME_SCREEN);
			  wlcm = 0;
			}
			UART_Con_Mash();
			break;
		case USBD_STATE_SUSPENDED:
			wlcm = 1;
			break;
	}
	return;
}




/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	GPIO_InitTypeDef GPIO_InitStruct = {0};


	int i;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  readConfig();
  if(SysCntrl.Magic!=0b10110){
	SysCntrl.MainFlash = 0;
	SysCntrl.cs0 = 3;
	SysCntrl.cs1 = 3;
	SysCntrl.Watchdog = 1;
	SysCntrl.PowerState = 1;
	SysCntrl.Magic = 0b10110;
	writeConfig();
  }

  SysCntrl.power_stage = 0;
  // GPIO extender address
  SysCntrl.i2c_bt[0] = 0x1;
  SysCntrl.i2c_bt[1] = 0;
  //SysCntrl.active_cs = 0;
  SysCntrl.rx_head = 0;
  SysCntrl.rx_tail = 0;
  console.idx = 0;
  for(i = 0;i<UART_BUF_SIZE;i++)
	  console.buf[i] = 0;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  char buf[10];
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_USB_DEVICE_Init();
  MX_ADC_Init();
  /* USER CODE BEGIN 2 */

  Set_CS(0);
  Set_CS(1);
  SFT_I2C_Init(GPIOA,GPIO_PIN_1,GPIOA,GPIO_PIN_0,&si2c1,0);
  SFT_I2C_Master_Transmit(&si2c1,GPIO_EXPANDER_ADDR,SysCntrl.i2c_bt,2,1); // Clear outputs
  SysCntrl.i2c_bt[0] = 0x3;
  SFT_I2C_Master_Transmit(&si2c1,GPIO_EXPANDER_ADDR,SysCntrl.i2c_bt,2,1); // All outputs
  SysCntrl.i2c_bt[0] = 0x1;

  SPI_Reset(0);
  SPI_Reset(1);


  USB_EnableGlobalInt(&hUsbDeviceFS);
  //HAL_GPIO_DeInit(GPIOB,GPIO_PIN_0|GPIO_PIN_3);

/*
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
*/
  	  //BKLSTOP

//  SetI2C_Mask(FLASH_EN_0);
//  ClrI2C_Mask(FLASH_EN_1);

  DisableSPI();


//  __enable_irq();


  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_RESET);
  UART_putstrln(WELCOME_SCREEN);
  //SetI2C_Mask(FLASH_EN_0);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  asm("nop");
	  if(SysCntrl.TimerTick) {
		  SysCntrl.TimerTick = 0;
		  switch(SysCntrl.MS_counter % 10) {
		  case 0:
			  main2();
			  break;
		  case 1:
			  //printBin(SysCntrl.BootByte);
			  if(SysCntrl.pgin == 0) {
				  SysCntrl.power_stage = 99;
			  }
			  PowerSM();
			  break;
		  case 2:
			  if((SysCntrl.pwrbtn==1) && (SysCntrl.power_stage%10==0))
				  SysCntrl.PowerState = 1;
			  break;
		  case 3:
			  break;
		  }

	  }

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14
                              |RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSICalibrationValue = 31;
  RCC_OscInitStruct.HSI14CalibrationValue = 31;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x2000090E;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PF1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA3 PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM17 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM17) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  SysCntrl.TimerTick = 1;
  if(++SysCntrl.MS_counter>999)SysCntrl.MS_counter = 0;


  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
