/***************************************************************************************
                                      声  明
    本项目代码仅供个人学习使用，可以自由移植修改，但必须保留此声明信息。移植过程中出现
其他不可估量的BUG，天际智联不负任何责任。请勿商用！

程序版本：2.1
更新日期：2018-1-23
程序作者：愤怒的小孩，东方萧雨
版权所有：西安天际智联信息技术有限公司
****************************************************************************************/
/**********************************************************************************
利用定时器TIM4的通道4的输出比较功能，给ADC1提供采样触发信号，这里触发周期为：100ms
触发周期计算公式为：psc/72*period/1000000（单位为秒）

主函数初始化代码为：
	TIM_OCTigrConfig();
**********************************************************************************/
#include "tim_octigr.h"

void TIM_OCTigrConfig(void)
{
	TIM_TimeBaseInitTypeDef TIM_timeBaseStucture;
	TIM_OCInitTypeDef TIM_ocInitStructure;
	
	u16 period 	= 10000;										//设置周期值100ms发送一次
	u16 pluse 	= 5000;											//设置CRR值，这里的值可以随便设置，不影响ADC采样周期，但要保证在正确的范围内
	
	//使能TIM4的时钟，TIM4是挂在APB1总线上的，注意TIM4的时钟是72M的
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	
	TIM_timeBaseStucture.TIM_ClockDivision = TIM_CKD_DIV1;      	//分频因子，输出给定时器的ETRP数字滤波器提供时钟
	TIM_timeBaseStucture.TIM_Prescaler = 720-1;                		//预分频，给TIMx_CNT驱动的时钟120
	TIM_timeBaseStucture.TIM_CounterMode = TIM_CounterMode_Up;		//向上计数
	TIM_timeBaseStucture.TIM_Period = period;						//设置周期，给ARR赋值
	TIM_TimeBaseInit(TIM4,&TIM_timeBaseStucture);
	
	//配置TIM4通道4的输出比较
	TIM_ocInitStructure.TIM_OCMode = TIM_OCMode_PWM1;				//PWM输出模式为PWM1
	TIM_ocInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;		//设置有效电平的极性
	TIM_ocInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//使能通道输出
	TIM_ocInitStructure.TIM_Pulse = pluse;							//设置PWM的脉冲宽度值，即CRR值
	TIM_OC4Init(TIM4,&TIM_ocInitStructure);
	
	TIM_ARRPreloadConfig(TIM4,ENABLE);								//使能TIM4的寄存器ARR的预装载功能，DISABLE时将会使改变ARR值时立即生效
	TIM_OC4PreloadConfig(TIM4,TIM_OCPreload_Enable);				//使能TIM4通道1的CCR的预装载功能，DISABLE时将会使改变CRR值时立即生效
	
	//使能TIM4定时器
	TIM_Cmd(TIM4,ENABLE);
}


