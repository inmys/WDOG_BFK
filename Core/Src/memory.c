#include "structs.h"
#include "main.h"
#include "memory.h"
#include "stdio.h"
#include "text.h"

extern SPI_HandleTypeDef hspi1;
extern void MX_SPI1_Init(void);

// Sector - 64
uint8_t SPI_ReadStatus() {
	Clr_CS(SysCntrl.active_cs);
	SysCntrl.spi_buf_tx[0] = 0x05;
	HAL_SPI_Transmit(&hspi1, SysCntrl.spi_buf_tx, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive(&hspi1,SysCntrl.spi_buf_rx, 1, HAL_MAX_DELAY);

	Set_CS(SysCntrl.active_cs);

	return SysCntrl.spi_buf_rx[0];
}

void Flash_WriteEnable() {

	Clr_CS(SysCntrl.active_cs);
	SysCntrl.spi_buf_tx[0] = 0x06;
	HAL_SPI_Transmit(&hspi1, SysCntrl.spi_buf_tx, 1, HAL_MAX_DELAY);
	Set_CS(SysCntrl.active_cs);
}

void SPI_PageRead(uint32_t spi_address, uint8_t *data, uint32_t len) {

	Clr_CS(SysCntrl.active_cs);
	SysCntrl.spi_buf_tx[0] = 0x0b;
	SysCntrl.spi_buf_tx[1] = (spi_address>>16) & 0xff;
	SysCntrl.spi_buf_tx[2] = (spi_address>>8)  & 0xff;
	SysCntrl.spi_buf_tx[3] = (spi_address>>0)  & 0xff;
	SysCntrl.spi_buf_tx[4] = 0;

	HAL_SPI_Transmit(&hspi1, SysCntrl.spi_buf_tx, 5, HAL_MAX_DELAY);

	HAL_SPI_Receive(&hspi1,data,len,HAL_MAX_DELAY);

	Set_CS(SysCntrl.active_cs);

}


void Flash_PageWrite() {
	do{

	}while(SPI_ReadStatus()&1);


		Flash_WriteEnable();
	if((SysCntrl.SPI_address & 0xffff) == 0)
		SPI_EraseAddr(SysCntrl.SPI_address);

		Flash_WriteEnable();


		Clr_CS(SysCntrl.active_cs);
		SysCntrl.spi_buf_tx[0] = 0x02;
		SysCntrl.spi_buf_tx[1] = (SysCntrl.SPI_address>>16) & 0xff;
		SysCntrl.spi_buf_tx[2] = (SysCntrl.SPI_address>>8)  & 0xff;
		SysCntrl.spi_buf_tx[3] = (SysCntrl.SPI_address>>0)  & 0xff;

		HAL_SPI_Transmit(&hspi1, SysCntrl.spi_buf_tx, 4, HAL_MAX_DELAY);
		HAL_SPI_Transmit(&hspi1, SysCntrl.SPI_page, SysCntrl.SPI_page_idx, HAL_MAX_DELAY);

		Set_CS(SysCntrl.active_cs);

		SysCntrl.SPI_address += SysCntrl.SPI_page_idx;

		SysCntrl.SPI_page_idx = 0;

}


void SPI_EraseAddr(uint32_t addr) {
	uint8_t i;

	Clr_CS(SysCntrl.active_cs);
	SysCntrl.spi_buf_tx[0] = 0xD8;
	SysCntrl.spi_buf_tx[1] = (addr>>16) & 0xff;
	SysCntrl.spi_buf_tx[2] = (addr>>8)  & 0xff;
	SysCntrl.spi_buf_tx[3] = (addr>>0)  & 0xff;

	HAL_SPI_Transmit(&hspi1, SysCntrl.spi_buf_tx, 4, HAL_MAX_DELAY);
	Set_CS(SysCntrl.active_cs);


	do{
		for(i=0;i<100;i++)asm("nop");
	}while(SPI_ReadStatus()&1);
}

void SPI_Reset(uint8_t cs) {
	Clr_CS(cs);
	SysCntrl.spi_buf_tx[0] = 0x66;
	SysCntrl.spi_buf_tx[1] = 0x99;
	HAL_SPI_Transmit(&hspi1, SysCntrl.spi_buf_tx, 2, HAL_MAX_DELAY);
	Set_CS(cs);
}

uint8_t SPI_ReadID(uint8_t cs,struct memoryReport *data) {
	Clr_CS(cs);
	SysCntrl.spi_buf_tx[0] = 0x9f;
	SysCntrl.spi_buf_tx[1] = 0x0;
	SysCntrl.spi_buf_tx[2] = 0x0;
	SysCntrl.spi_buf_tx[3] = 0x0;
	SysCntrl.spi_buf_tx[4] = 0x0;
	uint8_t result = HAL_SPI_TransmitReceive(&hspi1, SysCntrl.spi_buf_tx, SysCntrl.spi_buf_rx, 5,HAL_MAX_DELAY);

	if(result == HAL_OK && data!=NULL){
			data->ManufacturerID = SysCntrl.spi_buf_rx[1];
			data->MemoryType = SysCntrl.spi_buf_rx[2];
			data->MemoryCapacity = SysCntrl.spi_buf_rx[3];
			data->UniqID[0] = SysCntrl.spi_buf_rx[4];
			data->UniqID[1] = SysCntrl.spi_buf_rx[5];
	}
	Set_CS(cs);
	return result;
}

extern struct SConsole console;
void Xmodem_SPI(){
	uint8_t bt;
	int result,i;
	char buf[10];
	if(SysCntrl.TimerCnt) SysCntrl.TimerCnt--;

	switch(SysCntrl.XmodemState) {
	case XMODEM_STATE_INIT:
		result = ReadUartNonBlock(&bt,1);
		if((result>0)) {
			switch(bt){
			case 0x01:
				SysCntrl.XmodemState = XMODEM_STATE_S0;
				SysCntrl.X_idx = 0;

				SysCntrl.SPI_page_idx = 0;
				SysCntrl.bt_count = 128+4;
			break;
			case 0x03:
				SysCntrl.XmodemMode = 0;
				break;
			}
		} else {
			if(!result && !SysCntrl.TimerCnt) {
				SysCntrl.TryCounter--;
				if(!SysCntrl.TryCounter) {
					SysCntrl.XmodemMode = 0;
					UART_putstrln(1,"Timeout...");
				}
				else {
					SysCntrl.TimerCnt = XMODEM_TIME_1SEC;
					UART_putstrln(0,"C");
				}
			}
		}
		break;
	case XMODEM_STATE_S0: // SOH code received
		//uint8_t SPI_rxbuf[1024];
		//alt_u32 X_idx;
		//alt_u32 bt_count;
		result = ReadUartNonBlock(&SysCntrl.SPI_rxbuf[SysCntrl.X_idx],SysCntrl.bt_count);
		if(result>0) {
			SysCntrl.X_idx += result;
			SysCntrl.bt_count -= result;
			if(!SysCntrl.bt_count) {
				UART_SendByte(0x06); // ACK
				for(i=0;i<128;i++) {
					SysCntrl.SPI_page[SysCntrl.SPI_page_idx++] = SysCntrl.SPI_rxbuf[i+2];
				}
				if(SysCntrl.SPI_page_idx >= 255)
					Flash_PageWrite();
				SysCntrl.XmodemState = XMODEM_STATE_S1;
			}
		}
		break;
	case XMODEM_STATE_S1:
		result = ReadUartNonBlock(&bt,1);
		if(result>0){
			UART_putstrln(0,buf);
			if((bt == 0x01)) {
					SysCntrl.XmodemState = XMODEM_STATE_S0;
					SysCntrl.X_idx = 0;
					SysCntrl.bt_count = 128+4;
					}
			else
				if(bt == 0x04) {
					// Все страницы зашили
						SysCntrl.FWStatus = UPDATED;
						SysCntrl.BootAttempt = 3;
						writeConfig();
						UART_SendByte(0x06); // ACK
						for(;SysCntrl.SPI_page_idx<128; SysCntrl.SPI_page_idx++)
							SysCntrl.SPI_page[SysCntrl.SPI_page_idx] = 0;
						Flash_PageWrite();
						SysCntrl.XmodemState = XMODEM_STATE_INIT;
						SysCntrl.XmodemMode = 0;
						DisableSPI();
				}
				else
					if(bt == 0x17) {
						UART_SendByte(0x06); // ACK
						SysCntrl.XmodemState = XMODEM_STATE_INIT;
						SysCntrl.XmodemMode = 0;
						DisableSPI();
					}
					else
						if(bt == 0x18) {
							UART_SendByte(0x06); // ACK
							UART_SendByte(0x06); // ACK
							SysCntrl.XmodemState = XMODEM_STATE_INIT;
							SysCntrl.XmodemMode = 0;
							DisableSPI();

							UART_putstrln(1,"Canceled");
						}
			}

		break;
	case XMODEM_STATE_S2:
		break;
	case XMODEM_STATE_S3:
		break;
	}
}

void Xmodem_Init(){
	EnableSPI();
	SysCntrl.XmodemMode = 1;
	SysCntrl.XmodemState = XMODEM_STATE_INIT;
	SysCntrl.TryCounter = 100;
	SysCntrl.TimerCnt = XMODEM_TIME_1SEC;
	UART_putstrln(0,"C");
}





void FlashDump(uint8_t cs){
	int i;
	EnableSPI();
	//Switch_BootSpi2BMC();
	Clr_CS(cs);
	SysCntrl.spi_buf_tx[0] = 0x0b;
	SysCntrl.spi_buf_tx[1] = 0x0;
	SysCntrl.spi_buf_tx[2] = 0x0;
	SysCntrl.spi_buf_tx[3] = 0x0;
	SysCntrl.spi_buf_tx[4] = 0x0;

	HAL_SPI_Transmit(&hspi1, SysCntrl.spi_buf_tx,5,HAL_MAX_DELAY);

	if(HAL_SPI_Receive(&hspi1,SysCntrl.SPI_page, 256, HAL_MAX_DELAY) == HAL_OK){
		SPI_ReadID(0,NULL);
		UART_putstrln(0, SysCntrl.spi_buf_rx);
		for (i=0;i<256;i++){
			UART_SendByte(ByteToHEX(SysCntrl.SPI_page[i]>>4));
			UART_SendByte(ByteToHEX(SysCntrl.SPI_page[i]&0x0f));
			UART_putstrln(0,EMPTY_SYM);
		}
		UART_putstrln(1,"\n\r");
	}
	Set_CS(cs);
	DisableSPI();
}


void memoryMenu(){
	char buf[BUF_LEN];
	sprintf(buf,"CPU main flash #%d",SysCntrl.MainFlash+1);
	UART_putstrln(1,buf);
	clearBuf(buf);
	sprintf(buf,"CPU boot flash #%d",SysCntrl.BootFlash+1);
	UART_putstrln(1,buf);
	clearBuf(buf);
	sprintf(buf,"Boot attempt:%d/4",SysCntrl.BootAttempt+1);
	UART_putstrln(1,buf);
	clearBuf(buf);
	sprintf(buf,"Watchdog:%s",(SysCntrl.WatchdogConsole&&SysCntrl.WatchdogBootAlt)?SWT[0]:SWT[1]);
	UART_putstrln(1,buf);
	sprintf(buf,"Autoboot:%s",SysCntrl.Autoboot?"On":"Off by key");
	UART_putstrln(1,buf);
}

void writeConfig(){
	FLASH_EraseInitTypeDef EraseInitStruct;
	EraseInitStruct.TypeErase  = TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = CONFIG_ADDR_IN_FLASH-4;
	EraseInitStruct.NbPages = 1;

	// TODO: Error handler
	uint32_t error;
	HAL_FLASH_Unlock();
	HAL_FLASHEx_Erase(&EraseInitStruct,&error);
	uint16_t data = (SysCntrl.SavedConfigH<<8)|(SysCntrl.SavedConfigL);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,CONFIG_ADDR_IN_FLASH, data);
	HAL_Delay(50);
	HAL_FLASH_Lock();
}

void readConfig(){
	uint16_t *data = (uint16_t*) CONFIG_ADDR_IN_FLASH;

	SysCntrl.SavedConfigL = (*data)&(0b0000000011111111);
	SysCntrl.SavedConfigH = (*data)>>8;

}





