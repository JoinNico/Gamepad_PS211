#include "led.h"


// 初始化PB4、PB5为推挽输出
//-----------------------------------------------------------------------------------------
void LED_Init_JX(void)
{	
	GPIO_InitTypeDef  GPIO_InitStructure;					// 定义GPIO_InitTypeDef型结构体

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST,ENABLE);		// PB4的JTRST失能
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	// 使能GPIOB端口时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;	// Green--PB4、Blue--PB5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		// 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		// IO口速度为10MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					// 
	GPIO_SetBits(GPIOB,GPIO_Pin_4|GPIO_Pin_5);				// PB4、PB5输出高
}
//-----------------------------------------------------------------------------------------
