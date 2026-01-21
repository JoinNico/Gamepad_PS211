//
// Created by Administrator1 on 2025/11/14.
//

#ifndef JOY_H_
#define JOY_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "filter.h"


/* ==================== 配置参数 ==================== */

/* ADC相关参数 */
#define ADC_CENTER              2048    // ADC中心默认值（12位ADC）
#define ADC_MAX_RANGE           2048    // ADC最大有效范围（从中心到边缘）
#define CENTER_CALIBRATION_TOL  300     // 中心值自动校准容差（±300）

/* 滤波参数 */
#define FILTER_WINDOW_SIZE      10     // 移动平均滤波窗口大小（样本数）

/* 死区参数 */
#define CENTER_DEADZONE         100     // 中心死区（ADC值）消除中心抖动
#define EDGE_DEADZONE           50      // 边缘死区（ADC值）确保能达到最大值

/* 菜单控制参数 */
#define MENU_THRESHOLD          30      // 菜单触发阈值（-100到100）
#define MENU_FAST_TRIGGER_TIME  1000    // 进入快速模式的时间（毫秒）
#define MENU_FAST_INTERVAL      100     // 快速模式下的触发间隔（毫秒）

typedef struct {
    uint16_t x_raw;          // X轴滤波后的ADC值 (0-4095)
    uint16_t y_raw;          // Y轴滤波后的ADC值 (0-4095)
    float x_normalized;      // X轴归一化值 (-1.0 到 1.0)
    float y_normalized;      // Y轴归一化值 (-1.0 到 1.0)
    int16_t x_value;         // X轴整数值 (-100 到 100)
    int16_t y_value;         // Y轴整数值 (-100 到 100)
    int16_t throttle;        // 油门值（对数曲线处理后，-100到100）
} JoyStick_t;

void Joy_Init(void);
void Joy_Update(void);
int16_t Joy_GetThrottle(void);
JoyStick_t Joy_GetLeftStick(void);
JoyStick_t Joy_GetRightStick(void);
int8_t Joy_GetMenuControl(uint32_t delta_ms);
void Joy_GetCenterValues(uint16_t *left_x, uint16_t *left_y, uint16_t *right_x, uint16_t *right_y);


#ifdef __cplusplus
}
#endif

#endif /* JOY_H_ */