#ifndef _SI24R1_H
#define _SI24R1_H

#include "stm32f10x.h"
/**********************************************************************
宏定义区
***********************************************************************/
#define CE_LOW					GPIOB->BRR |= GPIO_Pin_0
#define CE_HIGH					GPIOB->BSRR |= GPIO_Pin_0

//==========================SI24R124L01============================================
#define TX_ADR_WIDTH    		5   		//这个地址是接收端的接收通道的地址的宽度
#define RX_ADR_WIDTH    		5   		//这个是本机接收通道0的地址宽度
#define TX_PLOAD_WIDTH  		32  		//要发送的有效数据长度，这个一般和接收端SI24R1的RX的FIFO设置值相对应
#define RX_PLOAD_WIDTH  		32  		//要接收的有效数据长度，这个决定RX端FIFO达到多少数据量后触发中断
//=========================SI24R124L01寄存器指令===================================
#define READ_REG_CMD        	0x00  		// 读寄存器指令
#define WRITE_REG_CMD       	0x20 		// 写寄存器指令
#define RD_RX_PLOAD     		0x61  		// 读取接收数据指令
#define WR_TX_PLOAD     		0xA0  		// 写待发数据指令
#define FLUSH_TX        		0xE1 		// 冲洗发送 FIFO指令
#define FLUSH_RX        		0xE2  		// 冲洗接收 FIFO指令
#define REUSE_TX_PL     		0xE3  		// 定义重复装载数据指令
#define NOP            			0xFF  		// 保留
//========================SPI(nRF24L01)寄存器地址===============================
#define CONFIG          		0x00  		// 配置收发状态，CRC校验模式以及收发状态响应方式
#define EN_AA           		0x01  		// 自动应答功能设置
#define EN_RXADDR       		0x02  		// 可用信道设置
#define SETUP_AW        		0x03  		// 收发地址宽度设置
#define SETUP_RETR      		0x04  		// 自动重发功能设置
#define RF_CH           		0x05  		// 工作频率设置
#define RF_SETUP        		0x06  		// 发射速率、功耗功能设置
#define STATUS          		0x07  		// 状态寄存器
#define OBSERVE_TX      		0x08  		// 发送监测功能
#define CD              		0x09  		// 地址检测           
#define RX_ADDR_P0      		0x0A  		// 频道0接收数据地址
#define RX_ADDR_P1      		0x0B  		// 频道1接收数据地址
#define RX_ADDR_P2      		0x0C  		// 频道2接收数据地址
#define RX_ADDR_P3      		0x0D  		// 频道3接收数据地址
#define RX_ADDR_P4      		0x0E  		// 频道4接收数据地址
#define RX_ADDR_P5      		0x0F  		// 频道5接收数据地址
#define TX_ADDR         		0x10  		// 发送地址寄存器
#define RX_PW_P0        		0x11 		// 接收频道0接收数据长度
#define RX_PW_P1        		0x12  		// 接收频道0接收数据长度
#define RX_PW_P2        		0x13  		// 接收频道0接收数据长度
#define RX_PW_P3        		0x14  		// 接收频道0接收数据长度
#define RX_PW_P4        		0x15  		// 接收频道0接收数据长度
#define RX_PW_P5        		0x16  		// 接收频道0接收数据长度
#define FIFO_STATUS     		0x17  		// FIFO栈入栈出状态寄存器设置
//=============================RF24l01状态=====================================

void SI24R1_Config(void);

void RX_Mode(void);
void SI24R1_SendPacket(u8* tfbuf);
void SI24R1_ReceivePacket(u8* rx_buf);
void WaitFlY_Connection(void);
void ReconnectionFly(void);
#endif

