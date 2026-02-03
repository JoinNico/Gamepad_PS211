#ifndef __OLED_H
#define __OLED_H

#include "main.h"

//--------------OLED参数定义---------------------
#define PAGE_SIZE    8
#define XLevelL		 0x00
#define XLevelH		 0x10
#define YLevel       0xB0
#define	Brightness	 0xFF
#define WIDTH 	     128
#define HEIGHT 	     64

//-------------写命令和数据定义-------------------
#define OLED_CMD     0	//写命令
#define OLED_DATA    1	//写数据

#define OLED_RST_Clr() 	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET)
#define OLED_RST_Set() 	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET)

#define OLED_CS_Clr()  	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET)
#define OLED_CS_Set()  	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET)


void OLED_Init(void);
void OLED_SendBuff(uint8_t buff[8][128]);
#endif
