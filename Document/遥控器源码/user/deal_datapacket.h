#ifndef _DEAL_DATAPACKET_H
#define _DEAL_DATAPACKET_H

#include "stm32f10x.h"

//每一位对应功能
#define GYRO_OFFSET 0x01 //第一位陀螺仪校准标志位
#define ACC_OFFSET 0x02  //第二位加速度校准标志位
#define BAR_OFFSET 0x04  //第三位气压计校准标志位
#define MAG_OFFSET 0x08  //第四位磁力计校准标志位
#define FLY_ENABLE 0x10  //第五位解锁上锁
#define WiFi_ONOFF 0x20  //第六位WiFi开关
#define FLY_MODE   0x40  //第七位模式选择(0:无头模式(默认) 1:有头模式)

#define GET_FLAG(FLAG)   (SENSER_OFFSET_FLAG&FLAG)==FLAG ? 1 : 0   //获取标志位状态

extern u8 SENSER_OFFSET_FLAG;

typedef struct
{
	u16 Thr;
	s16 Yaw;
	s16 Rol;
	s16 Pit;
	s16 Alt;
	s16 BattV;
}FLY_TYPE;

extern FLY_TYPE FLY;
void CAL(void);
void PackData(u8 firstByte);
void ReceiveDataAnalysis(void);
#endif
