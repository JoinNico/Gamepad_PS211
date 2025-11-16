/***************************************************************************************
                                      声  明
    本项目代码仅供个人学习使用，可以自由移植修改，但必须保留此声明信息。移植过程中出现
其他不可估量的BUG，天际智联不负任何责任。请勿商用！

程序版本：2.1
更新日期：2018-1-23
程序作者：愤怒的小孩，东方萧雨
版权所有：西安天际智联信息技术有限公司
****************************************************************************************/
/*******************************************************************************
利用TIM4通道4的输出比较触发ADC1开始采样，并且ADC各通道的数据转换完成后，触发DMA
请求，将ADC_DR中的数据存入内存数组中，DMA传输完成后，触发DMA传输完成中断
注意：
1.初始化ADC_DmaConfig()函数前，由于使用到TIM4的通道4的输出比较，所以要先初始化
TIM_OCTigrConfig()
2.DMA1_Channel1_IRQHandler()中断处理函数在senddata.c文件中

主函数初始化代码为：
	TIM_OCTigrConfig();
	ADC_DmaConfig();
********************************************************************************/
#include "adc_dma.h"
#include "paramsave.h"

u8 ADC_CALIBRATOR_OK;  //遥控通道ADC校准标志
//用来保存ADC各通道转换完成后的数据
vu16 ADC_ConvertedValue[4];  //遥控通道ADC值
vu16 ADC_Calibrator[4];      //遥控通道ADC校准值
void ADC_DmaConfig(void)
{
	GPIO_InitTypeDef GPIO_initStructure;
	DMA_InitTypeDef DMA_initStructure;
	ADC_InitTypeDef ADC_initStructure;
	
	//开启DMA1时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	
	//配置DMA1通道1，将ADC采样转换得到的数据传输到内存数组中
	DMA_initStructure.DMA_BufferSize = 4;								//每次传输的数据的个数，传输完时触发中断
	DMA_initStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					//传输方向为：外设->内存
	DMA_initStructure.DMA_M2M = DMA_M2M_Disable;								//失能内存到内存的传输方式
	DMA_initStructure.DMA_MemoryBaseAddr = (u32)ADC_ConvertedValue;				//数据保存到内存中数组的首地址（这里因为ADC_ConvertedValue是数组名，所以不用加&）
	DMA_initStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			//以16位为单位进行数据的传输
	DMA_initStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						//内存地址自增（这里地址是每次增加1，因为是以半字（16位）作为单位传输的）
	DMA_initStructure.DMA_Mode = DMA_Mode_Circular;								//循环传输的方式，这里必须为循环传输方式，否则会导致DMA只能传输一次
	DMA_initStructure.DMA_PeripheralBaseAddr = ((u32)&ADC1->DR);				//&ADC1->DR
	DMA_initStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//以半字为单位进行数据的传输
	DMA_initStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			//外设地址固定
	DMA_initStructure.DMA_Priority = DMA_Priority_Medium;						//DMA通道1的优先级设置为中级，（这个优先级是当同一个DMA的不同通道同时有传输数据的要求时，优先级高的先进行传输）
	DMA_Init(DMA1_Channel1,&DMA_initStructure);
	
	DMA_ITConfig(DMA1_Channel1,DMA_IT_TC,ENABLE);								//打开DMA通道1数据传输完成中断
	NVIC_EnableIRQ(DMA1_Channel1_IRQn);											//打开NVIC中对应的DMA通道1的中断通道
	
	//开启DMA1的通道1
	DMA_Cmd(DMA1_Channel1,ENABLE);
	
	
	//使能ADC时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_ADC1,ENABLE);
	
	GPIO_initStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;	//ADC1的通道0,1,2,3
	GPIO_initStructure.GPIO_Mode = GPIO_Mode_AIN;		//ADC输入管脚需要为模拟输入模式
	GPIO_Init(GPIOA,&GPIO_initStructure);
	
	//配置ADC1
	ADC_initStructure.ADC_ContinuousConvMode = DISABLE;			//单次采样模式，每次由TIM4的CCR触发采样开始
	ADC_initStructure.ADC_DataAlign = ADC_DataAlign_Right;					//数据对齐模式为：右对齐
	ADC_initStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T4_CC4;		//TIM4的通道4的CCR触发采样
	ADC_initStructure.ADC_Mode = ADC_Mode_Independent;							//各通道独立
	ADC_initStructure.ADC_NbrOfChannel = 4;									    //一共要采样的通道的数目
	ADC_initStructure.ADC_ScanConvMode = ENABLE; //打开扫描模式，由于这里有四个通道要采集，所以开始用扫描模式
	ADC_Init(ADC1,&ADC_initStructure);
	
	//开启ADC
	ADC_Cmd(ADC1,ENABLE);
	
	//开启ADC——DMA数据传输通道
	ADC_DMACmd(ADC1,ENABLE);
	
	//配置ADC采样参考时钟的预分频值
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);											
	
	//配置ADC的规则通道的采样顺序和采样时间
	ADC_RegularChannelConfig(ADC1,ADC_Channel_0,1,ADC_SampleTime_239Cycles5);	
	ADC_RegularChannelConfig(ADC1,ADC_Channel_1,2,ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_2,3,ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_3,4,ADC_SampleTime_239Cycles5);
	
	ADC_ResetCalibration(ADC1);										//重置ADC采样校准器，防止出现较大的误差
	while(ADC_GetCalibrationStatus(ADC1));							//等待校准成功
	ADC_StartCalibration(ADC1);										//开启ADC采样状态
	while(ADC_GetCalibrationStatus(ADC1));							//等到开启成功
	
	//使能外部触发ADC采样
	ADC_ExternalTrigConvCmd(ADC1,ENABLE);
	
//	ADC_Calibrator[0] = 1492;
//	ADC_Calibrator[1] = 1000;
//	ADC_Calibrator[2] = 1564;
//	ADC_Calibrator[3] = 1492;
}

void ADC_Calibration(void)
{
	static u8 cnt=0;
	static vs32 temp1=0,temp2=0,temp3=0,temp4=0;
	if(ADC_ConvertedValue[2]>1900&&ADC_ConvertedValue[2]<2196&&ADC_ConvertedValue[3]>1900&&ADC_ConvertedValue[3]<2196)
	{
		if(0==cnt)
		{
			temp1 = 0;
			temp2 = 0;
			temp3 = 0;
			temp4 = 0;
			cnt = 0;
		}
		temp1 += (1000 + ADC_ConvertedValue[0]*1000/4096);
		temp2 += (1000 + ADC_ConvertedValue[1]*500/4096);
		temp3 += (1000 + ADC_ConvertedValue[2]*1000/4096);
		temp4 += (1000 + ADC_ConvertedValue[3]*1000/4096);
		cnt++;
		if(cnt >= 60)
		{
			ADC_Calibrator[0] = temp1/cnt;
			ADC_Calibrator[1] = temp2/cnt;
			ADC_Calibrator[2] = temp3/cnt;
			ADC_Calibrator[3] = temp4/cnt;
			ADC_CALIBRATOR_OK = 0;
			cnt = 0;
			PID_WriteFlash();
		}
	}
}



