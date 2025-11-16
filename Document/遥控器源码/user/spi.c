/***************************************************************************************
                                      声  明
    本项目代码仅供个人学习使用，可以自由移植修改，但必须保留此声明信息。移植过程中出现
其他不可估量的BUG，天际智联不负任何责任。请勿商用！

程序版本：2.1
更新日期：2018-1-23
程序作者：愤怒的小孩，东方萧雨
版权所有：西安天际智联信息技术有限公司
****************************************************************************************/
/*********************************************************************
硬件SPI接口

注意：
1.此源文件必须结合nRF.c来使用，所以SPI_Config()初始化函数一般可以放在
nRF.c的初始化函数内，主函数不用单独初始化
*********************************************************************/
#include "spi.h"


/**********************************************************************
宏定义区
***********************************************************************/
#define CSN_LOW					GPIOA->BRR |= GPIO_Pin_4
#define CSN_HIGH				GPIOA->BSRR |= GPIO_Pin_4


u8 status;			//记录从STATUS寄存器中返回的状态值

void SPI_Config(void)
{
	SPI_InitTypeDef   SPI_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	//配置SPI的SCK，MISO和MOSI引脚为复用推挽模式
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	//配置CSN引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	//双线全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;						//SPI为主机模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;					//传输的数据长度为8位
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;							//SPI总线空闲时，sck线为低电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;						//在SCK的偶数边沿采样数据
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;							//NSS软件控制模式
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;	//时钟分频值，此处为8分频
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;					//数据高位先行
	SPI_InitStructure.SPI_CRCPolynomial = 7;							//CRC校验多项式设置，数据传输的最后一个字节出加上此处的7，以完成CRC校验
	SPI_Init(SPI1, &SPI_InitStructure);
	
	SPI_Cmd(SPI1, ENABLE);
}


/******************************************************************************
* 读/写一个字节的数据，这里默认用SPI1作为主机
* 输入参数txData:主机发送给从机的有效数据，或者只为了读取从机发送的数据，
* 				 主机发送给从机的无效的数据
* 返回值：从从机中读取的数据,或者从status寄存器中读取的状态值，（注意：
* 		  只有给从机写入操作指令+寄存器地址时，从机才会返回STATUS寄存器中的值，
* 		  写入其他的值时，从机不会返回STATUS寄存器中的值）
******************************************************************************/
u8 SPI_RW(u8 txData)
{
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==RESET);
	SPI_I2S_SendData(SPI1,txData);
	
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)==RESET);
	return SPI_I2S_ReceiveData(SPI1);
}


/**********************************************************************
* 用SPI给某器件的寄存器中写入一个字节的数据，即配置寄存器
* 输入参数reg：操作指令+寄存器地址
* 输入参数value:写入寄存器中的数据
***********************************************************************/
u8 SPI_Write_Byte(u8 reg, u8 value)
{
	CSN_LOW;					//拉低从机的片选NSS，从而选中该从机
	status = SPI_RW(reg);		//只有给从机写入操作指令+寄存器地址时，从机才会返回STATUS寄存器中的值，写入其他的值时，从机不会返回STATUS寄存器中的值
	SPI_RW(value);				
	CSN_HIGH;					//释放片选NSS，SPI总线空闲
	
	return status;
}


/**********************************************************************
* 用SPI给某器件的寄存器中写入多个字节的值
* 输入参数reg：操作指令+寄存器地址
* 输入参数*pBuf：要写入的数据保存区域首地址
* 输入参数len：要写入的数据的个数
***********************************************************************/
u8 SPI_Write_Buf(u8 reg, u8 *pBuf, u8 len)
{
	CSN_LOW;

	status = SPI_RW(reg);
	while(len)
	{
		SPI_RW(*pBuf);
		pBuf++;
		len--;
	}
	
	CSN_HIGH;
	
	return status;
}

/**********************************************************************
* 用SPI从某器件的寄存器中读取一个字节的数据
* 输入参数reg：操作指令+寄存器地址
* 返回值：从寄存器中读取的数据
***********************************************************************/
u8 SPI_Read_Byte(u8 reg)
{
	u8 value;
	CSN_LOW;
	SPI_RW(reg);
	value = SPI_RW(0);
	CSN_HIGH;
	
	return value;
}

/**********************************************************************
* 用SPI从某器件的寄存器中读取多个字节的数据
* 输入参数reg：操作指令+寄存器地址
* 输入参数*pBuf：读取到的数据保存区域首地址
* 输入参数len：要读取数据的个数
***********************************************************************/
u8 SPI_Read_Buf(u8 reg, u8 *pBuf, u8 len)
{
	CSN_LOW;
	
	status = SPI_RW(reg);
	while(len){
		*pBuf = SPI_RW(0);
		pBuf++;
		len--;
	}

	CSN_HIGH;
	
	return status;
}


