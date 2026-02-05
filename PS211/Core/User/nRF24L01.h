#ifndef __nRF24L01_H__
#define __nRF24L01_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "main.h"
#include "spi.h"

#define RF_CHANNEL 40 //设置射频通道

//nRF24L01 发送/接收、地址/数据宽度定义
#define TX_ADDR_WIDTH 5  //发送地址宽度设置为5个字节
#define RX_ADDR_WIDTH 5  //接收地址宽度设置为5个字节
#define TX_DATA_WIDTH 32 //发送数据宽度32个字节
#define RX_DATA_WIDTH 32 //接收数据宽度32个字节

//nRF24L01数据收发缓存区
typedef struct nRF24L01_Data_Buffer
{
  uint8_t Tx_Buf[TX_DATA_WIDTH];
  uint8_t Rx_Buf[RX_DATA_WIDTH];
} nRF24_DataStruct;

extern nRF24_DataStruct nRF24_data;
extern const uint8_t TX_ADDRESS[TX_ADDR_WIDTH];
extern const uint8_t RX_ADDRESS[RX_ADDR_WIDTH];

//nRF24L01 SPI 指令
#define R_REGISTER 0x00   //读配置寄存器，000A AAAA指出读操作的寄存器地址
#define W_REGISTER 0x20   //写配置寄存器，001A AAAA指出写操作寄存器地址(只在掉电模式和待机模式下可操作)
#define R_RX_PAYLOAD 0x61 //读RX有效数据：1-32字节(读操作从bit0开始，当读RX有效数据完成后，FIFO寄存器有效数据清除，应用于接收模式)
#define W_TX_PAYLOAD 0xA0 //写TX有效数据：1-32字节(写操作从bit0开始，应用于发射模式)
#define FLUSH_TX 0xE1     //清除TX的FIFO寄存器(应用于发射模式)
#define FLUSH_RX 0xE2     //清除RX的FIFO寄存器(应用于接收模式)
#define REUSE_TX_PL 0xE3  //重新使用上一包数据。当CE为高过程，数据包被不断的重新发送(在发射数据包过程中必须禁止数据包重利用功能)
#define NOP 0xFF          //空操作(可用来读状态寄存器)

//nRF24L01 寄存器地址
#define CONFIG 0x00      //配置寄存器地址(...(bit1=1上电,bit1=0掉电)、(bit0=1接收模式,bit0=0发射模式))
#define EN_AA_ES 0x01    //使能“自动应答”功能(此功能禁止后可与nRF24L01通讯)
#define EN_RXADDR 0x02   //接收地址允许
#define SETUP_AW 0x03    //设置地址宽度(所有数据通道)(bit[7:2]=00000、bit0,1组合('00'-无效)('01'-3字节宽度)('10'-4字节宽度)('11'-5字节宽度))
#define SETUP_RETR 0x04  //建立自动重发
#define RF_CH 0x05       //射频通道(bit7=0、bit[6:0]设置nRF24L01工作通道频率)
#define RF_SETUP 0x06    //射频寄存器
#define STATUS 0x07      //状态寄存器
#define OBSERVE_TX 0x08  //发送检测寄存器
#define CD 0x09          //载波检测寄存器'Carrier Detect'
#define RX_ADDR_P0 0x0A  //数据通道0接收地址。最大长度5个字节(先写低字节，所写字节数量由SETUP_AW设定)
#define RX_ADDR_P1 0x0B  //数据通道1接收地址。最大长度5个字节(先写低字节，所写字节数量由SETUP_AW设定)
#define RX_ADDR_P2 0x0C  //数据通道2接收地址。最低字节可设置，高字节部分必须与RX_ADDR_P1[39:8]相等
#define RX_ADDR_P3 0x0D  //数据通道3接收地址。最低字节可设置，高字节部分必须与RX_ADDR_P1[39:8]相等
#define RX_ADDR_P4 0x0E  //数据通道4接收地址。最低字节可设置，高字节部分必须与RX_ADDR_P1[39:8]相等
#define RX_ADDR_P5 0x0F  //数据通道5接收地址。最低字节可设置，高字节部分必须与RX_ADDR_P1[39:8]相等
#define TX_ADDR 0x10     //发送地址(先写低字节)在增强型ShockBurst模式下RX_ADDR_P0与此地址相等
#define RX_PW_P0 0x11    //(bit[7:6]默认为00、bit[5:0]表示接收数据通道0的有效数据宽度(1-32字节有效数据宽度))
#define RX_PW_P1 0x12    //(bit[7:6]默认为00、bit[5:0]表示接收数据通道1的有效数据宽度(1-32字节有效数据宽度))
#define RX_PW_P2 0x13    //(bit[7:6]默认为00、bit[5:0]表示接收数据通道2的有效数据宽度(1-32字节有效数据宽度))
#define RX_PW_P3 0x14    //(bit[7:6]默认为00、bit[5:0]表示接收数据通道3的有效数据宽度(1-32字节有效数据宽度))
#define RX_PW_P4 0x15    //(bit[7:6]默认为00、bit[5:0]表示接收数据通道4的有效数据宽度(1-32字节有效数据宽度))
#define RX_PW_P5 0x16    //(bit[7:6]默认为00、bit[5:0]表示接收数据通道5的有效数据宽度(1-32字节有效数据宽度))
#define FIFO_STATUS 0x17 //FIFO状态寄存器

//中断标志位
#define MAX_RT 0x10 //达到最大发送次数中断MAX_RT
#define TX_DS 0x20  //TX发送完成中断TX_DS
#define RX_DR 0x40  //接收到数据中断RX_DR

//nRF24L01 控制线电平状态定义
#define nRF24L01_CE(_x) HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, ((_x) ? GPIO_PIN_SET : GPIO_PIN_RESET))
#define nRF24L01_CS(_x) HAL_GPIO_WritePin(NRF_CS_GPIO_Port, NRF_CS_Pin, ((_x) ? GPIO_PIN_SET : GPIO_PIN_RESET))
#define nRF24L01_Read_IRQ() HAL_GPIO_ReadPin(NRF_IRQ_GPIO_Port, NRF_IRQ_Pin)
#define nRF24_Delay(_x) HAL_Delay(_x)

uint8_t nRF24_Check(void);
uint8_t nRF24_TXPacket(uint8_t *TXBuf);
uint8_t nRF24_RXPacket(uint8_t *RXBuf);
void nRF24_TX_Mode(void);
void nRF24_RX_Mode(void);

#ifdef __cplusplus
}
#endif

#endif // __nRF24L01_H__
