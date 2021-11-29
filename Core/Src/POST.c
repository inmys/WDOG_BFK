
#include "structs.h"
#include "POST.h"
#include "memory.h"
#include "i2c.h"
#include "power.h"
#include "stdio.h"

//extern I2C_HandleTypeDef hi2c1;
extern I2C_handler si2c1;

extern const char *CS_STASTUS_LABELS[];
void POST(){
	uint8_t i,j;
	char buf[BUF_LEN];
	char abuf[6],bbuf[6];
	//cbuf[30];
	memID conf;
	EnableSPI();

	for(i=0;i<2;i++){
		clearBuf(buf);
		for(j=0;j<4;j++){
				abuf[j] = 0;
				bbuf[j] = 0;
		}
		sprintf(buf,"Flash %d: ",i+1);
		UART_putstrln(0,buf);

		if(SPI_ReadID(i,&conf) == HAL_OK){
//			sprintf(cbuf,"\r\Manufacture: %x, MemCapacity: %x,MemType: %x, Uniq: %x",conf.ManufacturerID, conf.MemoryCapacity, conf.MemoryType, conf.UniqID);
//			UART_putstrln(cbuf);
			switch(conf.MemoryType){
			case 0xBA:
				sprintf(abuf,"3");
			break;
			case 0x60:
			case 0xBB:
				sprintf(abuf,"1.8");
			break;
			default:
				sprintf(abuf,"?");
			}

			switch(conf.MemoryCapacity){
			case 0x22:
				sprintf(bbuf,"2Gb");
			break;
			case 0x21:
				sprintf(bbuf,"1Gb");
			break;
			case 0x20:
				sprintf(bbuf,"512Mb");
			break;
			case 0x19:
				sprintf(bbuf,"256Mb");
			break;
			case 0x18:
				sprintf(bbuf,"128Mb");
			break;
			case 0x17:
				sprintf(bbuf,"64Mb");
			break;
			default:
				sprintf(bbuf,"?");
			}
		sprintf(buf,"%sV %s",abuf, bbuf);
		}
		else
			sprintf(buf,"FAILED");

		UART_putstrln(1,buf);
	}

	// I/O Expander
	clearBuf(buf);
	abuf[0] = 0x03;
	abuf[1] = 0b01010101;

	SFT_I2C_Master_Transmit(&si2c1,GPIO_EXPANDER_ADDR,abuf,2,1);

	uint8_t tester = 0x03;
	UART_putstrln(1,"I/O Expander: ");
	SFT_I2C_Master_Transmit(&si2c1,GPIO_EXPANDER_ADDR,&tester,1,1);
	tester = 0;
	SFT_I2C_Master_Receive(&si2c1,GPIO_EXPANDER_ADDR,&tester,1,1);
	if(tester==0b01010101)
		UART_putstrln(1,"OK");
	else
		UART_putstrln(1,"FAILED");
	abuf[1] = 0;
	SFT_I2C_Master_Transmit(&si2c1,GPIO_EXPANDER_ADDR,abuf,2,1);


	// Might be deleted cause POST run only while power on
	DisableSPI();


}

void clearBuf(char* buf){
	int j;
	for(j=0;j<BUF_LEN;j++)
		buf[j] = 0;
}
