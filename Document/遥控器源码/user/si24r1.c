/***************************************************************************************
                                      声  明
    本项目代码仅供个人学习使用，可以自由移植修改，但必须保留此声明信息。移植过程中出现
其他不可估量的BUG，天际智联不负任何责任。请勿商用！

程序版本：2.1
更新日期：2018-1-23
程序作者：愤怒的小孩，东方萧雨
版权所有：西安天际智联信息技术有限公司
****************************************************************************************/
/*************************************************************************************
nRF无线收发程序，以接收模式为主，当有数据要发送时，则切换到发射模式，并且发送完成后自动
切换回接收模式，等待接收数据

注意：
1.因为需要用到延迟函数，所以在SI24R1_Init()初始化前要先把systick_init()初始化
2.因为在EXTI中要用到SYSTICK的中断函数，所以需要配置NVIC，进行中断优先级的配置

主函数初始化代码为：
	SI24R1_Init();
***************************************************************************************/
#include "si24r1.h"
#include "spi.h"
#include "systick.h"
#include "usart.h"
#include "led.h"
#include "stdlib.h"
#include "oled.h"
#include "paramsave.h"
#include "adc_dma.h"
#include "oleddispaly.h"

u8  TX_ADDRESS[TX_ADR_WIDTH]= {0x34,0x43,0x10,0x10,0xFF};	//此地址用来识别接收端哪个RX通道可以接收发送出去的数据包
u8  RX_ADDRESS[RX_ADR_WIDTH]= {0x34,0x43,0x10,0x10,0xFF};	//此地址用来配置本机SI24R1的RX0通道的地址，同时为了能正常收到应答信号，此地址一般都和上面的地址配置相同

//发生中断时，根据STATUS寄存器中的值来判断是哪个中断源触发了IRQ中断
#define TX_DS			  0x20			  //数据发送完成中断
#define RX_DR			  0x40			  //数据接收完成中断
#define MAX_RT			  0x10		      //数据包重发次数超过设定值中断
#define AddrMax           50              //SI24R1最后一位地址最大值
//=====================================全局变量定义区========================================
vu8 sta;								  //接收从STATUS寄存器中返回的值
u8 RxBuf[RX_PLOAD_WIDTH];				  //数据接收缓冲区，数据发送缓冲区
u8 FLYDataRx_OK = 0;                      //飞机数据接收完成
u8 FLY_Connect_OK = 0;                    //遥控与飞机已连接
u8 Reconnection_flag = 0;                 //遥控与飞机已断开需要重连
extern vu8 ButtonMask;					  //用来保存哪个按键刚被按下了，原始定义在：button.c文件中
extern u16 TX_CNT ;                    	  //遥控数据发送计数，原始定义在：senddata.c文件中
extern u16 TX_ERROR ;                     //遥控数据发送失败计数，原始定义在：senddata.c文件中
extern u8 ADC_CALIBRATOR_OK;        //遥控通道ADC校准标志，原始文件在：adc_dma.h中
//========================================================================================



/**********************************************************************
初始化SI24R1
***********************************************************************/
void SI24R1_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructus;
	EXTI_InitTypeDef EXTI_initStructure;
	
	//初始化SPI接口
	SPI_Config();
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO,ENABLE);
	
	//初始化CE引脚
	GPIO_InitStructus.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructus.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructus.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructus);

	//配置IRQ引脚
	GPIO_InitStructus.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructus.GPIO_Mode = GPIO_Mode_IPU; //nRF中断产生时，IRQ引脚会被拉低，所以这里要配置成上拉输入
	GPIO_InitStructus.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructus);

	EXTI_initStructure.EXTI_Line = EXTI_Line1;
	EXTI_initStructure.EXTI_LineCmd = ENABLE;
	EXTI_initStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_initStructure.EXTI_Trigger = EXTI_Trigger_Falling;	//下降沿触发
	EXTI_Init(&EXTI_initStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource1);//开启GPIO管脚的中断线路
	NVIC_EnableIRQ(EXTI1_IRQn);

	CE_LOW;	//拉低CE，注意：读/写nRF寄存器均需要将CE拉低，使其进入待机或者掉电模式才可以
	
	//初始化SI24R1
	SPI_Write_Byte(WRITE_REG_CMD + SETUP_AW, 0x03);							//配置通信地址的长度，默认值时0x03,即地址长度为5字节
	SPI_Write_Buf(WRITE_REG_CMD + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);    	
	SPI_Write_Buf(WRITE_REG_CMD + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH); 	
	SPI_Write_Byte(WRITE_REG_CMD + SETUP_RETR, 0x1a); 						//自动重发延迟为500+86us，重发次数10次
	SPI_Write_Byte(WRITE_REG_CMD + EN_AA, 0x01);      						//接收数据后，只允许频道0自动应答
	SPI_Write_Byte(WRITE_REG_CMD + EN_RXADDR, 0x01);  						//只允许频道0接收数据
	
	SPI_Write_Byte(WRITE_REG_CMD + RF_SETUP, 0x27);   						//设置发射速率为2MHZ，发射功率为最大值0dB
	SPI_Write_Byte(WRITE_REG_CMD + RF_CH, 30);        						//设置通道通信频率，工作通道频率可由以下公式计算得出：Fo=（2400+RF-CH）MHz.并且射频收发器工作的频率范围从2.400-2.525GHz
	SPI_Write_Byte(WRITE_REG_CMD + RX_PW_P0, RX_PLOAD_WIDTH); 				//设置接收数据长度，本次设置为5字节，只有接收的数据达到此长度时，才会触发RX_DS中断
	
	SPI_Write_Byte(WRITE_REG_CMD + CONFIG, 0x0f);   						//默认处于接收模式

//	读回配置信息，防止配置出错（调试代码的时候可以使用，程序正常运行代码时，注释掉这些打印代码）
//	printf("SETUP_AW:%x\r\n",SPI_Read_Byte(READ_REG_CMD+SETUP_AW));
//	printf("SETUP_RETR:%x\r\n",SPI_Read_Byte(READ_REG_CMD+SETUP_RETR));
//	printf("EN_RXADDR:%x\r\n",SPI_Read_Byte(READ_REG_CMD+EN_RXADDR));
//	printf("EN_AA:%x\r\n",SPI_Read_Byte(READ_REG_CMD+EN_AA));
//	printf("RF_SETUP:%x\r\n",SPI_Read_Byte(READ_REG_CMD+RF_SETUP));
//	printf("RF_CH:%x\r\n",SPI_Read_Byte(READ_REG_CMD+RF_CH));
//	printf("RX_PW_P0:%x\r\n",SPI_Read_Byte(READ_REG_CMD+RX_PW_P0));
//	printf("CONFIG:%x\r\n",SPI_Read_Byte(READ_REG_CMD+CONFIG));

	CE_HIGH;
}

//IRQ引脚对应的EXTI中断处理函数
void EXTI1_IRQHandler(void)
{
	CE_LOW;											//拉低CE，以便读取SI24R1中STATUS中的数据
	sta = SPI_Read_Byte(READ_REG_CMD+STATUS);		//读取STATUS中的数据，以便判断是由什么中断源触发的IRQ中断
	
	if(sta & TX_DS)                              	//数据发送成功，并且收到了应答信号
	{										
		RX_Mode();								 	//将SI24R1的模式改为接收模式，等待接收数据
		if(ButtonMask & 0x01)
		{
			LED_On(LED1);						 	//LED1常亮表示飞机已经解锁成功，并正在进行数据通讯
		}else
		{
			LED_Toggle(LED1);					 	//LED1闪烁表示飞机加锁模式中，但是和飞机通讯成功
		}
		SPI_Write_Byte(WRITE_REG_CMD+STATUS,TX_DS);	//清除TX_DS中断
		SPI_Write_Byte(FLUSH_TX,0xff);              //清空TX_FIFO
	}
	else if(sta & RX_DR)                           	//数据接收成功
	{	
		FLY_Connect_OK	= 1;		                //飞机与遥控器已连接
		SI24R1_ReceivePacket(RxBuf);				//将数据从RX端的FIFO中读取出来
		FLYDataRx_OK 	= 1;
		SPI_Write_Byte(WRITE_REG_CMD+STATUS,RX_DR);	//清除RX_DR中断
	}
	else if(sta & MAX_RT)                          	//触发了MAX_RT中断
	{		
		if(TX_ERROR++>100)                          //遥控数据丢帧计数
		{
			Reconnection_flag = 1;			        //重连标志置位
			FLY_Connect_OK = 0 ;                    //飞机与遥控器断开连接
		}
		LED_Off(LED1);									  //LED1灭时，表示和飞机通讯中断
		RX_Mode();										  //将SI24R1的模式改为接收模式，等待接收数据
		SPI_Write_Byte(WRITE_REG_CMD+STATUS,MAX_RT);	  //清除MAX_RT中断
		SPI_Write_Byte(FLUSH_TX,0xff);                    //清空TX_FIFO
//		printf("MAX_RT OK!!!\r\n");
	}
	EXTI_ClearITPendingBit(EXTI_Line1);
}


/**********************************************************************
配置SI24R1为RX模式，准备开始接收数据
***********************************************************************/
void RX_Mode(void)
{
	CE_LOW;											//拉低CE，进入待机模式，准备开始往SI24R1中的寄存器中写入数据
	
	SPI_Write_Byte(WRITE_REG_CMD + CONFIG, 0x0f); 	//配置为接收模式
	SPI_Write_Byte(WRITE_REG_CMD + STATUS, 0x7e);	//写0111 xxxx 给STATUS，清除所有中断标志，防止一进入接收模式就触发中断
	
	CE_HIGH; 									    //拉高CE，准备接受从外部发送过来的数据
}

/**********************************************************************
从SI24R1的RX的FIFO中读取一组数据包
输入参数rx_buf:FIFO中读取到的数据的保存区域首地址
***********************************************************************/
void SI24R1_ReceivePacket(u8* rx_buf)
{
	CE_LOW;
	SPI_Read_Buf(RD_RX_PLOAD,rx_buf,RX_PLOAD_WIDTH);	//从RX端的FIFO中读取数据，并存入指定的区域，注意：读取完FIFO中的数据后，SI24R1会自动清除其中的数据
	SPI_Write_Byte(FLUSH_RX,0xff);                   	//清除接收FIFO(很必要)
	CE_HIGH;											//重新拉高CE，让其重新处于接收模式，准备接收下一个数据
}

/**********************************************************************
配置SI24R1为TX模式，并发送一个数据包
输入参数tfbuf:即将要发送出去的数据区首地址
***********************************************************************/
void SI24R1_SendPacket(u8* tfbuf)
{
	CE_LOW;																	//拉低CE，进入待机模式，准备开始往SI24R1中的寄存器中写入数据
	
//	SPI_Write_Buf(WRITE_REG_CMD + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); 	//装载接收端地址，由于这里只有一个通道通讯，不用改变接收端的SI24R1的接收通道地址,所以，这句可以注释掉
	SPI_Write_Buf(WR_TX_PLOAD, tfbuf, TX_PLOAD_WIDTH); 			//将数据写入TX端的FIFO中,写入的个数与TX_PLOAD_WIDTH设置值相同
	
	SPI_Write_Byte(WRITE_REG_CMD + CONFIG, 0x0e); 				//将SI24R1配置成发射模式
	SPI_Write_Byte(WRITE_REG_CMD + STATUS, 0x7e);				//写0111 xxxx 给STATUS，清除所有中断标志，防止一进入发射模式就触发中断
	
	CE_HIGH;													//拉高CE，准备发射TX端FIFO中的数据
	
	delay_ms(1);												//CE拉高后，需要延迟至少130us
}

/* 遥控器飞机对频（其实是对地址） */
void WaitFlY_Connection(void)
{
	static u8 cnt = 0,preaddr;
	ConnectingDisplay();//断线连接状态显示
	while(1)
	{
		if(FLY_Connect_OK) 
		{
			cnt = 0;
			if(preaddr != TX_ADDRESS[TX_ADR_WIDTH-1])
			{
				PID_WriteFlash();  
				//保存上一次连接的飞机的SI24R1地址
//				printf("Address save :%d preaddr:%d\r\n",TX_ADDRESS[4],preaddr);
			}
//			printf("Fly connect OK!!!\r\n");
			return;
		}else if(cnt++ < 10)
		{
			PID_ReadFlash();   //读取上一次保存的飞机的SI24R1地址
			preaddr = TX_ADDRESS[TX_ADR_WIDTH-1];
			SI24R1_Config();
			delay_ms(50);
//			printf("Flash read SI24R1addr:%d\r\n",TX_ADDRESS[4]);
		}
		else
		{
			TX_ADDRESS[TX_ADR_WIDTH-1]++ ;
			RX_ADDRESS[TX_ADR_WIDTH-1]++ ;
			if(TX_ADDRESS[TX_ADR_WIDTH-1]>AddrMax && RX_ADDRESS[TX_ADR_WIDTH-1]>AddrMax)
			{
				TX_ADDRESS[TX_ADR_WIDTH-1] = 0x00;
				RX_ADDRESS[TX_ADR_WIDTH-1] = 0x00;
			}
			SPI_Write_Buf(WRITE_REG_CMD + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);    	
			SPI_Write_Buf(WRITE_REG_CMD + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH); 
			delay_ms(100);
		}
		if(ADC_CALIBRATOR_OK)
		{
			OLED_ShowString(byte(2),line4,"Calibration",8);
		}
		else
		{
			OLED_ShowString(byte(2),line4,"Connecting...",8);
		}
	}
}

//断线重连函数
void ReconnectionFly(void)
{
	if(Reconnection_flag)
	{
		ConnectingDisplay();
		if(FLY_Connect_OK)        //遥控已重新连接
		{
			Display_init();
			Reconnection_flag = 0;  //断线重连标志复位            
			return;
		}
		SI24R1_Config();
	}
}


