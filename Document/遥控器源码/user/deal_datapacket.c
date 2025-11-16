/***************************************************************************************
                                      声  明
    本项目代码仅供个人学习使用，可以自由移植修改，但必须保留此声明信息。移植过程中出现
其他不可估量的BUG，天际智联不负任何责任。请勿商用！

程序版本：2.1
更新日期：2018-1-23
程序作者：愤怒的小孩，东方萧雨
版权所有：西安天际智联信息技术有限公司
****************************************************************************************/
/****************************************************************************************
数据的打包和解包
*****************************************************************************************/
#include "si24r1.h"
#include "deal_datapacket.h"
#include "usart.h"

FLY_TYPE FLY;

u8 SENSER_OFFSET_FLAG;

u8 packetData[12];		//打包后待发送的数据包，这个长度必须和接收端SI24R1的接收通道定义的有效数据长度相同，否则接收不到数据
u8 dataPID = 0;			//数据包识别PID	
vu16 accelerator = 0;	//记录油门上一次的数值，用于下拉油门的时候，防止动力损失过大造成的失衡

extern u8 RxBuf[RX_PLOAD_WIDTH];	
extern vu8 ButtonMask;				//用来保存哪个按键刚被按下了，原始定义在：button.c文件中
extern vu16 ADC_ConvertedValue[4];	//用来保存ADC各通道转换完成后的数据，原始定义在：adc_dma.c中
extern u8 ADC_CALIBRATOR_OK;        //遥控通道ADC校准标志，原始文件在：adc_dma.h中
extern vu16 ADC_Calibrator[4];      //遥控通道ADC校准值，原始定义在：adc_dma.c中
vu16 ADC_Value[4];
/**************************************************************************************
打包数据

注意按照下面的通讯协议进行打包数据：以字节为单位：
前导码-按键MASK--ADC1低8--ADC1高8--ADC2低8--ADC2高8--ADC3低8--ADC3高8--ADC4低8--ADC4高8--数据包标识--校验码0xa5
其中：前导码只有0x01和0x08才表示有效的数据包，0x01表示此数据包是由ADC采样完成触发的，
0x08表示此数据包是由遥控器上的按键触发的，
数据包标识用于接收端SI24R1识别是否是同一数据包的作用（这在飞机上主要用于当遥控信号中断时，自动开始降落。）
**************************************************************************************/

vu16 ADC_ValueLimit(vu16 value,vu16 L,vu16 R,vu16 min,vu16 max)
{
	vu16 result = 0;
	if(value>L&&value<R) 
		value= 1500;
	if(value < L)
		value += 1500-L;
	if(value > R)
		value -= 1500-L;
	
	value = (value<=min)?min:value;//防止越界
	value = (value>=max)?max:value;//防止越界

	result = value;
	return result;
}

void PackData(u8 firstByte)
{
//	printf("ADC_Converonver1:%d\r\n",ADC_ConvertedValue[0]);
//	printf("ADC_Converonver12:%d\r\n",ADC_ConvertedValue[1]);
//	printf("ADC_Converonver13:%d\r\n",ADC_ConvertedValue[2]);
//	printf("ADC_Converonver14:%d\r\n",ADC_ConvertedValue[3]);
//	printf("+++++++++++++++++++++++++++++++++++++++++++++++\r\n");
	
	
	ADC_Value[0] = 1500 + ((1000+ADC_ConvertedValue[0]*1000/4096) - ADC_Calibrator[0]);
	ADC_Value[0] = ADC_ValueLimit(ADC_Value[0],1470,1530,1100,1900);

	ADC_Value[1] = (ADC_ConvertedValue[1]*1000/4096); //油门0~1000;
	
	ADC_Value[2] = 1500 + ((1000+(ADC_ConvertedValue[2])*1000/4096) - ADC_Calibrator[2] );
	ADC_Value[2] = ADC_ValueLimit(ADC_Value[2],1470,1530,1100,1900);
	
	ADC_Value[3] = 1500 + ((1000+(ADC_ConvertedValue[3])*1000/4096) - ADC_Calibrator[3] );
	ADC_Value[3] = ADC_ValueLimit(ADC_Value[3],1470,1530,1100,1900);


//	printf("ADC1:%d\r\n",ADC_Value[0]);
//	printf("ADC2:%d\r\n",ADC_Value[1]);
//	printf("ADC3:%d\r\n",ADC_Value[2]);
//	printf("ADC4:%d\r\n",ADC_Value[3]);
//	
//	printf("ADC_Calibrator1:%d\r\n",ADC_Calibrator[0]);
//	printf("ADC_Calibrator2:%d\r\n",ADC_Calibrator[1]);
//	printf("ADC_Calibrator3:%d\r\n",ADC_Calibrator[2]);
//	printf("ADC_Calibrator4:%d\r\n",ADC_Calibrator[3]);
//	
//	printf("==================================\r\n");
	
	
	//数据包识别PID自增，并且超过200时自动归零
	if(dataPID>=200){
		dataPID = 0;
	}else{
		dataPID++;
	}
	
//=========直接采用指针操作内存中的数值将16位转成8位，速度快，并且不会发生精度截取的现象，还要注意，STM32是小端地址============
	packetData[0] = firstByte;					  //前导码
	packetData[1] = ButtonMask;                   //按键
	packetData[2] = *(((u8*)ADC_Value)+0);        //YAW角
	packetData[3] = *(((u8*)ADC_Value)+1);
	packetData[4] = *(((u8*)ADC_Value)+2);		  //油门数据打包
	packetData[5] = *(((u8*)ADC_Value)+3);
	packetData[6] = *(((u8*)ADC_Value)+4);        //ROLL角
	packetData[7] = *(((u8*)ADC_Value)+5);
	packetData[8] = *(((u8*)ADC_Value)+6);        //PITCH角
	packetData[9] = *(((u8*)ADC_Value)+7);
	packetData[10] = dataPID;					  //这个非常重要，这是防止飞机逃脱遥控的保证
	packetData[11] = 0xa5;						  //校验码：1010 0101
}

//====================================将处理好的数据进行打包===================================
//==============采用移位方式进行数据打包，注意，这种方式有可能会发生精度截取的现象=============
//	packetData[0] = firstByte;							//前导码
//	packetData[1] = ButtonMask;
//	packetData[2] = (u8)(ADC_ConvertedValue[0]>>8);
//	packetData[3] = ADC_ConvertedValue[0];
//	packetData[4] = (u8)(ADC_ConvertedValue[1]>>8);
//	packetData[5] = ADC_ConvertedValue[1];
//	packetData[6] = (u8)(ADC_ConvertedValue[2]>>8);
//	packetData[7] = ADC_ConvertedValue[2];
//	packetData[8] = (u8)(ADC_ConvertedValue[3]>>8);
//	packetData[9] = ADC_ConvertedValue[3];
//	packetData[10] = 0xa5;								//校验码：1010 0101
	

//接收飞机发送过来的数值
void ReceiveDataAnalysis(void)
{
	if(RxBuf[0]==0xFF )
	{
		SENSER_OFFSET_FLAG = RxBuf[1];
		FLY.Thr = ((RxBuf[2]<<8)|RxBuf[3]);
		FLY.Yaw = ((s16)(RxBuf[4]<<8)|RxBuf[5])/100;
		FLY.Pit = ((s16)(RxBuf[6]<<8)|RxBuf[7])/100;
		FLY.Rol = ((s16)(RxBuf[8]<<8)|RxBuf[9])/100;
		FLY.Alt = ((s16)(RxBuf[10]<<8)|RxBuf[11])/100;
		FLY.BattV = ((s16)(RxBuf[12]<<8)|RxBuf[13]);
//		printf("SENSER_OFFSET_FLAG:0x%x\r\n",SENSER_OFFSET_FLAG);
//		printf("Thr:%d Yaw:%d Rol:%d Pit:%d\r\n",Thr,Yaw,Rol,Pit);
	}
}

void CAL(void)
{
	if(ADC_ConvertedValue[2]<6 && ADC_ConvertedValue[3]<6 && ADC_ConvertedValue[1]<6)
	{
		ADC_CALIBRATOR_OK = 1;
//		printf("遥控器校准！\r\n");
	}
}


