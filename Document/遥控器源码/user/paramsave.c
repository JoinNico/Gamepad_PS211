/***************************************************************************************
                                      声  明
    本项目代码仅供个人学习使用，可以自由移植修改，但必须保留此声明信息。移植过程中出现
其他不可估量的BUG，天际智联不负任何责任。请勿商用！

程序版本：2.1
更新日期：2018-1-23
程序作者：愤怒的小孩，东方萧雨
版权所有：西安天际智联信息技术有限公司
****************************************************************************************/
#include "paramsave.h"
#include "flash.h"
#include "usart.h"

#define INT160(dwTemp)       (*(int16_t*)(&dwTemp))
#define INT161(dwTemp)       (*((int16_t *)(&dwTemp) + 1))
#define INT162(dwTemp)       (*((int16_t *)(&dwTemp) + 2))
#define INT163(dwTemp)       (*((int16_t *)(&dwTemp) + 3))

SAVE_TYPE flash;
extern u8  TX_ADDRESS[5];
extern u8  RX_ADDRESS[5];
extern vu16 ADC_Calibrator[4];      //遥控通道ADC校准值，原始定义在：adc_dma.c中
extern u8 ADC_CALIBRATOR_OK;

void ParamsToTable(void)
{
	flash.TXaddr = TX_ADDRESS[4];
	flash.RXaddr = RX_ADDRESS[4];
	flash.YAW = ADC_Calibrator[0];
	flash.THR = ADC_Calibrator[1];
	flash.ROL = ADC_Calibrator[2];
	flash.PIT = ADC_Calibrator[3];
}

void TableToParams(void)
{
	TX_ADDRESS[4] = flash.TXaddr;
	RX_ADDRESS[4] = flash.RXaddr;
	ADC_Calibrator[0] = flash.YAW;
	ADC_Calibrator[1] = flash.THR;
	ADC_Calibrator[2] = flash.ROL;
	ADC_Calibrator[3] = flash.PIT;
}

void ParamsClearAll(void)
{
	flash.TXaddr = 0;
	flash.RXaddr = 0;
	flash.YAW = 0xFF;
	flash.THR = 0xFF;
	flash.ROL = 0xFF;
	flash.PIT = 0xFF;
}

void PID_ClearFlash(void)
{
	u8 size;
	ParamsClearAll();//数据清除
	size = sizeof(flash)/sizeof(uint16_t);//保存的数据长度
	STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)(&flash),size);
}

void PID_WriteFlash(void)
{
	u8 size;
	ParamsToTable();//浮点数转换成整数
	size = sizeof(flash)/sizeof(uint16_t);//保存的数据长度
	STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)(&flash),size);
}

void PID_ReadFlash(void)
{
	u8 size;
	size = sizeof(flash)/sizeof(uint16_t);
	STMFLASH_Read(FLASH_SAVE_ADDR,(u16*)(&flash),size);
	TableToParams();//将整数转换成浮点数
}

void Flash_DataRead(void)
{
	PID_ReadFlash();
	if(ADC_Calibrator[0]==0xFFFF && ADC_Calibrator[1]==0xFFFF && ADC_Calibrator[2]==0xFFFF && ADC_Calibrator[3]==0xFFFF)
	{
		ADC_CALIBRATOR_OK = 1;
	}else
	{
		ADC_CALIBRATOR_OK = 0;
	}
}


