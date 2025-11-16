/***************************************************************************************
                                      声  明
    本项目代码仅供个人学习使用，可以自由移植修改，但必须保留此声明信息。移植过程中出现
其他不可估量的BUG，天际智联不负任何责任。请勿商用！

程序版本：2.1
更新日期：2018-1-23
程序作者：愤怒的小孩，东方萧雨
版权所有：西安天际智联信息技术有限公司
****************************************************************************************/
/*************************************************************************
系统滴答定时器
使用前，需要初始化计时基准
注意：systick_Init()函数中就已经将systick中断打开了
**************************************************************************/
#include "systick.h"

u32 count;

void SysTick_Init(void)
{
	SysTick->LOAD  = (uint32_t)(SystemCoreClock/1000000 - 1UL);
	SysTick->VAL   = 0UL;
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |SysTick_CTRL_TICKINT_Msk;
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}


void delay_us(u32 time)
{
	if(time<=0)
		return;

	count = time;
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	while(count!=0);
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void delay_ms(u32 time)
{
	if(time<=0)
		return;

	count = time*1000;
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	while(count!=0);
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

//systick中断处理函数
void SysTick_Handler(void)
{
	if(count!=0){
		count--;
	}
}

