/***************************************************************************************
                                      声  明
    本项目代码仅供个人学习使用，可以自由移植修改，但必须保留此声明信息。移植过程中出现
其他不可估量的BUG，天际智联不负任何责任。请勿商用！

程序版本：2.1
更新日期：2018-1-23
程序作者：愤怒的小孩，东方萧雨
版权所有：西安天际智联信息技术有限公司
****************************************************************************************/
/***********************************************************************************
这里设置所有中断的优先级
注意：
1.当需要中断嵌套时，优先级高的可以打断优先级低的中断，实现中断嵌套
2.在优先级设置中，以Preemption为主，数值越小，优先级越高，但是当Preemption数值一样时，
Subpriority数值越小，优先级越高；只有在Preemption和Subpriority的数值相同时才是同一级的
中断，不会发生中断嵌套，并且直接使用内核的方法配置中断优先级是没有响应优先级的说法的，
要想配置响应优先级，请使用stm32的优先级配置方法，可以配置抢占优先级和响应优先级
*************************************************************************************/

#include "nvic.h"

//中断优先级组
#define NVIC_PRIORITY_GROUP_3          3 /* Preemption: 4 bits / Subpriority: 0 bits                        */
#define NVIC_PRIORITY_GROUP_4          4 /* Preemption: 3 bits / Subpriority: 1 bits                        */
#define NVIC_PRIORITY_GROUP_5          5 /* Preemption: 2 bits / Subpriority: 2 bits                        */
#define NVIC_PRIORITY_GROUP_6          6 /* Preemption: 1 bits / Subpriority: 3 bits                        */
#define NVIC_PRIORITY_GROUP_7          7 /* Preemption: 0 bits / Subpriority: 4 bits                        */


//为所有用到的中断配置抢占优先级和响应优先级
void NVIC_Config(void)
{
	//SysTick_IRQn中断由于在有些中断函数中也会使用到，所以它的优先级是最高的
	NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_PRIORITY_GROUP_5, 0, 0)); 			//SysTick_IRQn中断
	NVIC_SetPriority(EXTI1_IRQn, NVIC_EncodePriority(NVIC_PRIORITY_GROUP_5, 1, 0));				//EXTI1_IRQn中断，SI24R1的IRQ引脚中断
	NVIC_SetPriority(DMA1_Channel1_IRQn, NVIC_EncodePriority(NVIC_PRIORITY_GROUP_5, 1, 1));		//DMA1_Channel1_IRQn中断，DMA将ADC数据传输完成中断
	NVIC_SetPriority(EXTI15_10_IRQn, NVIC_EncodePriority(NVIC_PRIORITY_GROUP_5, 2, 2));			//EXTI15_10_IRQn中断,按键触发中断（优先级必须小于EXTI1_IRQn否则会导致SI24R1l连接中断）
}

