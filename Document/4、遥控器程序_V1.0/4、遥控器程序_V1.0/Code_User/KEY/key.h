#ifndef __KEY_H
#define __KEY_H


#include "stm32f10x.h"
#include "bit_band.h"
#include "delay.h"

// 12个按键的状态：按下为1，松开为0
//-------------------------------
extern u8 F_KEY_Down[12];
//-------------------------------


// 按键输入宏定义
//------------------------------------------------------------------------
#define 	WKUP_in					PA_in(0)	// WKUP--PA0
#define 	KEY1_in					PC_in(14)	// KEY_1--PC14

#define 	KEY_UP_in				PA_in(11)	// KEY_UP--PA11
#define 	KEY_DOWN_in				PA_in(12)	// KEY_DOWN--PA12
#define 	KEY_LEFT_in				PA_in(15)	// KEY_LEFT--PA15
#define 	KEY_RIGHT_in			PB_in(3)	// KEY_RIGHT--PB3

#define 	KEY_A_in				PB_in(6)	// KEY_A  -- PB6
#define 	KEY_AA_in				PB_in(7)	// KEY_A+ -- PB7
#define 	KEY_B_in				PB_in(8)	// KEY_B  -- PB8
#define 	KEY_BB_in				PB_in(9)	// KEY_B+ -- PB9


#define 	KEY_Rocker1_in			PA_in(8)	// KEY_LeftRocker--PA8
#define 	KEY_Rocker2_in			PC_in(15)	// KEY_RightRocker--PC15
//------------------------------------------------------------------------

// 按键序号
//--------------------------------------------------------------
#define 	WKUP				0				// 唤醒键
#define 	KEY1				1				// 按键1
#define 	KEY_UP				2				// 上
#define 	KEY_DOWN			3				// 下
#define 	KEY_LEFT			4				// 左
#define 	KEY_RIGHT			5				// 右
#define 	KEY_A				6				// A
#define 	KEY_AA				7				// A+
#define 	KEY_B				8				// B
#define 	KEY_BB				9				// B+

#define 	KEY_Rocker1			10				// 摇杆1的按键
#define 	KEY_Rocker2			11				// 摇杆2的按键


// 按键初始化
//----------------------------
void KEY_Init_JX(void);

// 按键扫描函数
//----------------------------
void RF2G4_KEY_Scan_JX(void);


#endif	/* __KEY_H */
