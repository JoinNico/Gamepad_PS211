/***************************************************************************************
                                      声  明
    本项目代码仅供个人学习使用，可以自由移植修改，但必须保留此声明信息。移植过程中出现
其他不可估量的BUG，天际智联不负任何责任。请勿商用！

程序版本：2.1
更新日期：2018-1-23
程序作者：愤怒的小孩，东方萧雨
版权所有：西安天际智联信息技术有限公司
****************************************************************************************/
#include "stm32f10x.h"
#include "led.h"
#include "systick.h"
#include "usart.h"
#include "spi.h"
#include "si24r1.h"
#include "nvic.h"
#include "adc_dma.h"
#include "tim_octigr.h"
#include "deal_datapacket.h"
#include "button.h"
#include "oled.h"
#include "oleddispaly.h"


extern u8 ADC_CALIBRATOR_OK; 

int main(void)
{
	USART_Config();
	printf("usart is ready\r\n");
	SysTick_Init();
	OLED_Init();
	NVIC_Config();
	BUTTON_Config();
	LED_Config();
	LED_On(LED1);
	SI24R1_Config();
	TIM_OCTigrConfig();
	ADC_DmaConfig();	//这个必须最后一个配置，因为一旦配置好这个，ADC就会开始工作了，则DMA会开始每个一段时间产生中断，或者先关闭总中断，最后所有都配置完毕后在打开总中断
	WaitFlY_Connection();
	Display_init();
	
	while(1)
	{
		Display_Update();//OLED更新显示
		ReconnectionFly();//断线重连
	}
}


