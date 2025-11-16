#ifndef MONI_FLASH_H
#define MONI_FLASH_H
#include "stm32f10x.h"

typedef struct SAVE_TYPE
{
	u16 TXaddr;
	u16 RXaddr;
	vu16 YAW;
	vu16 THR;
	vu16 ROL;
	vu16 PIT;
	
}SAVE_TYPE;

void TableToParams(void);
void ParamsToTable(void);
void ParamsClearAll(void);
void PID_WriteFlash(void);
void PID_ReadFlash(void);
void PID_ClearFlash(void);
	
#endif
