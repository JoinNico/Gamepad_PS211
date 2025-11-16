#ifndef _LED_H
#define _LED_H

#include "stm32f10x.h"

#define LED1 	GPIO_Pin_13


void LED_Config(void);
void LED_On(u16 LEDx);
void LED_Off(u16 LEDx);
void LED_Toggle(u16 LEDx);
void LED_CyclieOn(u32 delayms);

#endif
