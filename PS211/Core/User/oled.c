#include "oled.h"
#include "string.h"
#include "spi.h"

/*******************************************************************
 * @name       :void OLED_WR_Byte(unsigned dat,unsigned cmd)
 * @date       :2018-08-27
 * @function   :Write a byte of content to the OLED screen
 * @parameters :dat:Content to be written
                cmd:0-write command
					1-write data
 * @retvalue   :None
********************************************************************/
void OLED_WR_Byte(unsigned dat,unsigned cmd)
{
	// 准备传输的数据 (3-wire SPI格式)
	// 16位数据: bit15=D/C#, bit14-bit7=dat, bit6-bit0=don't care
	uint16_t tx_data = ((cmd & 0x01) << 15) | ((dat & 0xFF) << 7);
	uint16_t rx_data = 0;

	OLED_CS_Clr();
	// 注意：参数1表示传输1个16位数据（2个字节）
	HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)&tx_data, (uint8_t*)&rx_data, 1, 100);
	OLED_CS_Set();
}

/*******************************************************************
 * @name       :void OLED_Reset(void)
 * @date       :2018-08-27
 * @function   :Reset OLED screen
 * @parameters :dat:0-Display full black
                    1-Display full white
 * @retvalue   :None
********************************************************************/
void OLED_Reset(void)
{
	OLED_RST_Set();
	HAL_Delay(100);
	OLED_RST_Clr();
	HAL_Delay(100);
	OLED_RST_Set();
}

/*******************************************************************
 * @name       :void OLED_Init(void)
 * @date       :2018-08-27
 * @function   :initialise OLED SH1106 control IC
 * @parameters :None
 * @retvalue   :None
********************************************************************/
void OLED_Init(void)
{
 	HAL_Delay(200);
	OLED_Reset();     //复位OLED

/**************初始化SSD1306*****************/
	OLED_WR_Byte(0xAE,OLED_CMD); /*display off*/
	OLED_WR_Byte(0x00,OLED_CMD); /*set lower column address*/
	OLED_WR_Byte(0x10,OLED_CMD); /*set higher column address*/
	OLED_WR_Byte(0x40,OLED_CMD); /*set display start line*/
	OLED_WR_Byte(0xB0,OLED_CMD); /*set page address*/
	OLED_WR_Byte(0x81,OLED_CMD); /*contract control*/
	OLED_WR_Byte(0xFF,OLED_CMD); /*128*/
	OLED_WR_Byte(0xA1,OLED_CMD); /*set segment remap*/
	OLED_WR_Byte(0xA6,OLED_CMD); /*normal / reverse*/
	OLED_WR_Byte(0xA8,OLED_CMD); /*multiplex ratio*/
	OLED_WR_Byte(0x3F,OLED_CMD); /*duty = 1/64*/
	OLED_WR_Byte(0xC8,OLED_CMD); /*Com scan direction*/
	OLED_WR_Byte(0xD3,OLED_CMD); /*set display offset*/
	OLED_WR_Byte(0x00,OLED_CMD);
	OLED_WR_Byte(0xD5,OLED_CMD); /*set osc division*/
	OLED_WR_Byte(0x80,OLED_CMD);
	OLED_WR_Byte(0xD9,OLED_CMD); /*set pre-charge period*/
	OLED_WR_Byte(0XF1,OLED_CMD);
	OLED_WR_Byte(0xDA,OLED_CMD); /*set COM pins*/
	OLED_WR_Byte(0x12,OLED_CMD);
	OLED_WR_Byte(0xDB,OLED_CMD); /*set vcomh*/
	OLED_WR_Byte(0x30,OLED_CMD);
	OLED_WR_Byte(0x8D,OLED_CMD); /*set charge pump disable*/
	OLED_WR_Byte(0x14,OLED_CMD);
	OLED_WR_Byte(0xAF,OLED_CMD); /*display ON*/
}

/*******************************************************************
 * @name       :OLED_SendBuff
 * @brief      :发送整个帧缓冲区到OLED
 * @param      buff:8x128的帧缓冲区
 * @retval     None
********************************************************************/
void OLED_SendBuff(uint8_t buff[PAGE_SIZE][WIDTH])
{
	for(uint8_t page = 0; page < PAGE_SIZE; page++)
	{
		OLED_WR_Byte (YLevel + page,OLED_CMD);    // 设置页地址（0~7）(b0-b7)
		OLED_WR_Byte(XLevelL,OLED_CMD);		// 设置显示位置—列低地址
		OLED_WR_Byte (XLevelH,OLED_CMD);       // 设置显示位置—列高地址
		// 发送当前页的所有列数据
		for(uint16_t width = 0; width < WIDTH; width++)
		{
			OLED_WR_Byte(buff[page][width], OLED_DATA);
		}
	}
}



























