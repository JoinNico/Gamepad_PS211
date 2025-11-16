#ifndef _ADC_DMA_H
#define _ADC_DMA_H

#include "stm32f10x.h"
extern vu16 ADC_Calibrator[4];      //遥控通道ADC校准值

void ADC_DmaConfig(void);
void ADC_Calibration(void);

#endif
