/***************************************************************************************
                                      声  明
    本项目代码仅供个人学习使用，可以自由移植修改，但必须保留此声明信息。移植过程中出现
其他不可估量的BUG，天际智联不负任何责任。请勿商用！

程序版本：2.1
更新日期：2018-1-23
程序作者：愤怒的小孩，东方萧雨
版权所有：西安天际智联信息技术有限公司
****************************************************************************************/
/*****************************************************************************
将打包好的数据包发送出去
触发数据发送的有：按键中断，用于传输ADC采样的DMA传输完成中断

注意：
1.由于用到了几个中断函数，所以要在nvic.c文件中配置好他们的中断优先级顺序，
2.这个源文件，只需将其头文件包含进main.c文件中即可，无需在主函数中加入初始化函数
****************************************************************************/
#include "senddata.h"
#include "systick.h"
#include "button.h"
#include "si24r1.h"
#include "led.h"
#include "deal_datapacket.h"
#include "usart.h"
#include "adc_dma.h"

u16 TX_CNT = 0;                      //遥控数据发送计数
u16 TX_ERROR = 0;                    //遥控数据发送失败计数
float TX_ERROR_PERCENT = 0;          //遥控数据丢包率
extern u8 FLY_Connect_OK;            //飞机数据接收完成
extern u8 Reconnection_flag;         //遥控与飞机已连接
extern u8 ADC_CALIBRATOR_OK;         //遥控通道ADC校准标志，原始文件在：adc_dma.h中
extern u8 packetData[11];			 //打包后待发送的数据包，原始定义在：nRF.c文件中
extern vu8 ButtonMask;				 //标记哪个按钮被点击了，原始定义在：button.c文件中
extern vu16 ADC_ConvertedValue[4];	 //用来保存ADC各通道转换完成后的数据，原始定义在：adc_dma.c中
//按键中断处理函数
void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line14) != RESET){
		if((GPIOB->IDR & GPIO_Pin_14) == RESET){				//注意：这里不能用==SET来判断，因为SET = !RESET,也就是说SET的数字代表值是非零
			delay_ms(10);										//延迟20ms
			if((GPIOB->IDR & GPIO_Pin_14) == RESET){			//再检测一下按键的电平状态，若此时还是处于低电平状态，说明按键不是由于抖动触发的
				while((GPIOB->IDR & GPIO_Pin_14) == RESET);		//等待按钮释放
				if(ADC_ConvertedValue[1]>=100)					//油门拉倒最低，触发KEY1才是有效的
					return;
				ButtonMask ^= 0x01;														//按键1第一次被按下时为0x01，再次被按下时为0x00，以此重复
				PackData(0x08);									//打包数据，并且前导码为0x08
				SI24R1_SendPacket(packetData);
			}
		}
		EXTI_ClearITPendingBit(EXTI_Line14);					//清除EXTI中断标志
	}else if(EXTI_GetITStatus(EXTI_Line15) != RESET){
		if((GPIOB->IDR & GPIO_Pin_15) == RESET){
			delay_ms(10);										//延迟20ms
			if((GPIOB->IDR & GPIO_Pin_15) == RESET){			//再检测一下按键的电平状态，若此时还是处于低电平状态，说明按键不是由于抖动触发的
				while((GPIOB->IDR & GPIO_Pin_15) == RESET);
				ButtonMask ^= 0x02;								//按键2被按下
				PackData(0x08);
				SI24R1_SendPacket(packetData);
			}
		}
		EXTI_ClearITPendingBit(EXTI_Line15);
	}else if(EXTI_GetITStatus(EXTI_Line13) != RESET){
		if((GPIOB->IDR & GPIO_Pin_13) == RESET){
			delay_ms(10);										//延迟20ms
			if((GPIOB->IDR & GPIO_Pin_13) == RESET){			//再检测一下按键的电平状态，若此时还是处于低电平状态，说明按键不是由于抖动触发的
				while((GPIOB->IDR & GPIO_Pin_13) == RESET);
				ButtonMask |= 0x04;								//按键3被按下
				PackData(0x08);
				SI24R1_SendPacket(packetData);
			}
			ButtonMask &= ~0x04;
		}
		EXTI_ClearITPendingBit(EXTI_Line13);
	}else if(EXTI_GetITStatus(EXTI_Line12) != RESET){
		if((GPIOB->IDR & GPIO_Pin_12)== RESET){
			delay_ms(10);										//延迟20ms
			if((GPIOB->IDR & GPIO_Pin_12)== RESET){				//再检测一下按键的电平状态，若此时还是处于低电平状态，说明按键不是由于抖动触发的
				while((GPIOB->IDR & GPIO_Pin_12)== RESET);
				ButtonMask ^= 0x08;								//按键4被按下
				PackData(0x08);
				SI24R1_SendPacket(packetData);
			}
		//	ButtonMask &= ~0x08;
		}
		EXTI_ClearITPendingBit(EXTI_Line12);
	}
}
//求遥控数据丢包率
void Get_TxErrorPercent(void)
{
		if(TX_CNT++ == 100) //满100清零
		{
			TX_CNT = 0; 
			TX_ERROR = 0;
		}
		TX_ERROR_PERCENT = (float)TX_ERROR/(float)TX_CNT;
		if(TX_ERROR_PERCENT == 1)
		{
			Reconnection_flag = 1;			             //重连标志置位
			FLY_Connect_OK = 0 ;                         //飞机与遥控器断开连接
		}
//		printf("TX_CNT:%d TX_ERROR:%d baifen:%0.2f\r\n",TX_CNT,TX_ERROR,TX_ERROR_PERCENT);
}

//DMA1通道1中断通道处理函数，触发此中断时，说明已经将数据从ADC转移到内存数组中，可以触发数据发送了
void DMA1_Channel1_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC1)==SET)
	{
		CAL();	
		if(ADC_CALIBRATOR_OK)
		{
			ADC_Calibration();
		}
		PackData(0x01);							//打包数据，并且前导码为0x01
		SI24R1_SendPacket(packetData);
		Get_TxErrorPercent();                  	//求遥控数据丢包率
//		printf("BattV:%0.2f\r\n",ADC_ConvertedValue[4]*3.3*2/4096);
		DMA_ClearITPendingBit(DMA1_IT_TC1);		//清除DMA1通道1传输完成中断
	}
}


