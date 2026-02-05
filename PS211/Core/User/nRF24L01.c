#include "nRF24L01.h"

static uint8_t nRF24_SPI_TransmitReceive(uint8_t TXdata);
static uint8_t nRF24_Read_Reg(uint8_t RegAddr);
static uint8_t nRF24_Write_Reg(uint8_t RegAddr, uint8_t Wdata);
static uint8_t nRF24_Read_Buffer(uint8_t RegAddr, uint8_t *pRXBuf, uint8_t ByteLen);
static uint8_t nRF24_Write_Buffer(uint8_t RegAddr, uint8_t *pTXBuf, uint8_t ByteLen);

nRF24_DataStruct nRF24_data = {0};
const uint8_t TX_ADDRESS[TX_ADDR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0x01}; //发送器件地址
const uint8_t RX_ADDRESS[RX_ADDR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0x01}; //发送器件地址

/**
 * @function: static uint8_t nRF24_SPI_TransmitReceive(uint8_t TXdata)
 * @description: nRF24L01通过SPI读写
 * @param {uint8_t TXdata} 待写入的数据
 * @return: RXdata 返回的数据
 */
static uint8_t nRF24_SPI_TransmitReceive(uint8_t TXdata)
{
  uint8_t RXdata = 0;
  HAL_SPI_TransmitReceive(&hspi2, &TXdata, &RXdata, 1, 1000);
  return RXdata;
}

/**
 * @function: static uint8_t nRF24_Read_Reg(uint8_t RegAddr)
 * @description: 读取某个寄存器中的数据
 * @param {uint8_t RegAddr} 待读取的寄存器地址
 * @return: RegData 读取的寄存器状态数据
 */
static uint8_t nRF24_Read_Reg(uint8_t RegAddr)
{
  uint8_t RegData;
  nRF24L01_CS(0);
  nRF24_SPI_TransmitReceive(RegAddr);
  RegData = nRF24_SPI_TransmitReceive(NOP);
  nRF24L01_CS(1);
  return RegData;
}

/**
 * @function: static uint8_t nRF24_Write_Reg(uint8_t RegAddr, uint8_t Wdata)
 * @description: 向某个寄存器中写入数据
 * @param {uint8_t RegAddr} 待写入的寄存器地址
 * @param {uint8_t Wdata} 待发送的数据
 * @return: Rstatus 返回状态寄存器的数据
 */
static uint8_t nRF24_Write_Reg(uint8_t RegAddr, uint8_t Wdata)
{
  uint8_t Rstatus;
  nRF24L01_CS(0);
  Rstatus = nRF24_SPI_TransmitReceive(RegAddr);
  nRF24_SPI_TransmitReceive(Wdata);
  nRF24L01_CS(1);
  return Rstatus;
}

/**
 * @function: static uint8_t nRF24_Read_Buffer(uint8_t RegAddr, uint8_t *pRXBuf, uint8_t ByteLen)
 * @description: 读取寄存器缓冲区中多个字节数据
 * @param {uint8_t RegAddr} 待读取的寄存器地址
 * @param {uint8_t *pRXBuf} 存储读取到的数据
 * @param {uint8_t ByteLen} 读取数据长度
 * @return: Rstatus 返回状态寄存器的数据
 */
static uint8_t nRF24_Read_Buffer(uint8_t RegAddr, uint8_t *pRXBuf, uint8_t ByteLen)
{
  uint8_t Rstatus, i;
  nRF24L01_CS(0);
  Rstatus = nRF24_SPI_TransmitReceive(RegAddr);
  for (i = 0; i < ByteLen; i++)
    pRXBuf[i] = nRF24_SPI_TransmitReceive(NOP);
  nRF24L01_CS(1);
  return Rstatus;
}

/**
 * @function: static uint8_t nRF24_Write_Buffer(uint8_t RegAddr, uint8_t *pTXBuf, uint8_t ByteLen)
 * @description: 写入多个字节到寄存器缓冲区
 * @param {uint8_t RegAddr} 待写入的寄存器地址
 * @param {uint8_t *pTXBuf} 写入的数据
 * @param {uint8_t ByteLen} 写入数据长度
 * @return: Rstatus 返回状态寄存器的数据
 */
static uint8_t nRF24_Write_Buffer(uint8_t RegAddr, uint8_t *pTXBuf, uint8_t ByteLen)
{
  uint8_t Rstatus, i;
  nRF24L01_CS(0);
  Rstatus = nRF24_SPI_TransmitReceive(RegAddr);
  for (i = 0; i < ByteLen; i++)
    nRF24_SPI_TransmitReceive(*pTXBuf++);
  nRF24L01_CS(1);
  return Rstatus;
}

/**
 * @function: uint8_t nRF24_Check(void)
 * @description: 检测nRF4L01工作是否正常
 * @param {void}
 * @return: 0 成功
 * @return: 1 失败
 */
uint8_t nRF24_Check(void)
{
  uint8_t i;
  uint8_t TXCheckdata[5] = {0x55, 0x55, 0x55, 0x55, 0x55};
  uint8_t RXCheckdata[5];
  nRF24_Write_Buffer(W_REGISTER + TX_ADDR, TXCheckdata, 5);
  nRF24_Read_Buffer(TX_ADDR, RXCheckdata, 5);
  for (i = 0; i < 5; i++)
  {
    if (RXCheckdata[i] != 0x55)
      break;
  }
  if (i == 5)
    return 0;
  else
    return 1;
}

/**
 * @function: uint8_t nRF24_TXPacket(uint8_t *TXBuf)
 * @escription: 向nRF24L01的发送缓冲区 写入数据并启动发送
 * @param {uint8_t *TXBuf} 待写入发送的数据
 * @return: MAX_RT 达到最大发送次数
 * @return: 0 发送完成
 * @return: 1 其他原因发送失败
 */
uint8_t nRF24_TXPacket(uint8_t *TXBuf)
{
  uint8_t Rstatus;
  nRF24L01_CE(0);
  nRF24_Write_Buffer(W_TX_PAYLOAD, TXBuf, TX_DATA_WIDTH); //写数据到发送缓冲区TXBUF 32字节
  nRF24L01_CE(1);                                         //启动发送
  while (nRF24L01_Read_IRQ() != 0)
    ;                                            //等待发送完成
  Rstatus = nRF24_Read_Reg(STATUS);              //读取状态寄存器的值
  nRF24_Write_Reg(W_REGISTER + STATUS, Rstatus); //清除TX_DS或MAX_RT中断标志
  //判断产生的中断类型
  if (Rstatus & MAX_RT) //达到最大重发次数
  {
    nRF24_Write_Reg(FLUSH_TX, NOP); //清除TX FIFO缓冲区
    return MAX_RT;
  }
  else if (Rstatus & TX_DS) //发送完成
    return 0;
  else
    return 1; //其他原因发送失败
}

/**
 * @function: uint8_t nRF24_RXPacket(uint8_t *RXBuf)
 * @description: 启动nRF24L01并从接收缓冲区读取接收到的数据
 * @param {uint8_t *RXBuf} 保存读取的数据
 * @return: 0 收到数据
 * @return: 1 未收到数据
 */
uint8_t nRF24_RXPacket(uint8_t *RXBuf)
{
  uint8_t Rstatus;
  Rstatus = nRF24_Read_Reg(STATUS);              //读取状态寄存器的值
  nRF24_Write_Reg(W_REGISTER + STATUS, Rstatus); //清除TX_DS或MAX_RT中断标志
  if (Rstatus & RX_DR)                           //接收到数据
  {
    nRF24_Read_Buffer(R_RX_PAYLOAD, RXBuf, RX_DATA_WIDTH); //读取数据
    nRF24_Write_Reg(FLUSH_RX, NOP);                        //清除RX FIFO寄存器
    return 0;
  }
  return 1; //没有收到数据
}

/**
 * @function: void nRF24_TX_Mode(void)
 * @description: 将nRF24L01设置为发射模式
 * @param {void}
 * @return: void
 */
void nRF24_TX_Mode(void)
{
  nRF24L01_CE(0);
  nRF24_Write_Buffer(W_REGISTER + TX_ADDR, (uint8_t *)TX_ADDRESS, TX_ADDR_WIDTH);    //写TX器件地址
  nRF24_Write_Buffer(W_REGISTER + RX_ADDR_P0, (uint8_t *)RX_ADDRESS, RX_ADDR_WIDTH); //设置RX器件地址,为了应答接收设备，接收通道0地址和发送地址相同

  nRF24_Write_Reg(W_REGISTER + EN_AA_ES, 0x01);    //使能通道0的自动应答
  nRF24_Write_Reg(W_REGISTER + EN_RXADDR, 0x01);   //使能通道0的接收地址
  nRF24_Write_Reg(W_REGISTER + SETUP_RETR, 0x1A);  //设置自动重发间隔时间:500us + 86us,最大自动重发次数:10次
  nRF24_Write_Reg(W_REGISTER + RF_CH, RF_CHANNEL); //设置RF通道为RF_CHANNEL
  nRF24_Write_Reg(W_REGISTER + RF_SETUP, 0x0F);    //设置数据传输速率2Mbps,发射功率0dBm,低噪声放大增益开启
  nRF24_Write_Reg(W_REGISTER + CONFIG, 0x0E);      //配置为中断可用  ，CRC使能，16位CRC校验，上电模式，发送模式
  // nRF24_Write_Reg(W_REGISTER + STATUS, 0x7E);            //清除中断，防止干扰
  nRF24L01_CE(1);
  nRF24_Delay(1);
}

/**
 * @function: void nRF24_RX_Mode(void)
 * @description: 将nRF24L01设置为接收模式
 * @param {void}
 * @return: void
 */
void nRF24_RX_Mode(void)
{
  nRF24L01_CE(0);
  nRF24_Write_Buffer(W_REGISTER + RX_ADDR_P0, (uint8_t *)RX_ADDRESS, RX_ADDR_WIDTH); //设置RX器件地址

  nRF24_Write_Reg(W_REGISTER + EN_AA_ES, 0x01);          //使能通道0的自动应答
  nRF24_Write_Reg(W_REGISTER + EN_RXADDR, 0x01);         //使能通道0的接收地址
  nRF24_Write_Reg(W_REGISTER + RF_CH, RF_CHANNEL);       //设置RF通道为RF_CHANNEL
  nRF24_Write_Reg(W_REGISTER + RX_PW_P0, RX_DATA_WIDTH); //选择接收通道0的有效数据宽度32
  nRF24_Write_Reg(W_REGISTER + RF_SETUP, 0x0F);          //设置数据传输速率2Mbps,发射功率0dBm,低噪声放大增益开启
  nRF24_Write_Reg(W_REGISTER + CONFIG, 0x0F);            //配置为中断可用，CRC使能，16位CRC校验，上电模式，接收模式
  // nRF24_Write_Reg(W_REGISTER + STATUS, 0x7E);            //清除中断，防止干扰
  nRF24L01_CE(1);
  nRF24_Delay(1);
}
