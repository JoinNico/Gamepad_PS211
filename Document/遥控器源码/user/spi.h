#ifndef _SPI_H
#define _SPI_H

#include "stm32f10x.h"

void SPI_Config(void);
u8 SPI_RW(u8 txData);

u8 SPI_Write_Byte(u8 reg, u8 value);
u8 SPI_Write_Buf(u8 reg, u8 *pBuf, u8 len);
u8 SPI_Read_Byte(u8 reg);
u8 SPI_Read_Buf(u8 reg, u8 *pBuf, u8 len);


#endif

