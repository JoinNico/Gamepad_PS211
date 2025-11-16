//
// Created by Administrator1 on 2025/11/14.
//

#ifndef __JOY_H__
#define __JOY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

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

/* ==================== 数据结构 ==================== */

/**
 * @brief 摇杆数据结构
 */
typedef struct {
    uint16_t x_raw;          // X轴滤波后的ADC值 (0-4095)
    uint16_t y_raw;          // Y轴滤波后的ADC值 (0-4095)
    float x_normalized;      // X轴归一化值 (-1.0 到 1.0)
    float y_normalized;      // Y轴归一化值 (-1.0 到 1.0)
    int16_t x_value;         // X轴整数值 (-100 到 100)
    int16_t y_value;         // Y轴整数值 (-100 到 100)
    int16_t throttle;        // 油门值（对数曲线处理后，-100到100）
} JoyStick_t;

typedef struct {
    uint16_t buffer[FILTER_WINDOW_SIZE];
    uint32_t sum;
    uint8_t index;
    uint8_t filled;
} FilterState_t;

/* ==================== 函数声明 ==================== */

/**
 * @brief 摇杆初始化
 * @note 必须在使用前调用，会启动ADC DMA采集
 *       自动检测摇杆中心位置并校准（允许±CENTER_CALIBRATION_TOL偏差）
 *       如果检测到的中心值偏离ADC_CENTER超过容差，则使用默认值
 *       初始化滤波缓冲区为中心值
 */
void Joy_Init(void);

/**
 * @brief 更新摇杆数据
 * @note 应在主循环或定时器中定期调用，建议10-50ms调用一次
 *
 * 处理流程：
 * 1. 读取原始ADC值
 * 2. 移动平均滤波（FILTER_WINDOW_SIZE个样本）
 * 3. 应用中心死区（CENTER_DEADZONE）
 * 4. 应用圆周死区（EDGE_DEADZONE）并归一化到[-1, 1]
 * 5. Y轴油门应用对数曲线
 */
void Joy_Update(void);

/**
 * @brief 获取左摇杆油门值（对数曲线）
 * @return 油门值 -100到100（向上为正，向下为负）
 * @note 油门使用对数曲线，低速时更灵敏，高速时更平滑
 */
int16_t Joy_GetThrottle(void);

/**
 * @brief 获取左摇杆完整数据
 * @return 左摇杆数据结构
 * @note 包含原始值、归一化值、整数值和油门值
 */
JoyStick_t Joy_GetLeftStick(void);

/**
 * @brief 获取右摇杆完整数据
 * @return 右摇杆数据结构
 * @note 包含原始值、归一化值、整数值
 */
JoyStick_t Joy_GetRightStick(void);

/**
 * @brief 菜单控制处理
 * @param delta_ms 距离上次调用的时间间隔（毫秒）
 * @return 菜单移动命令：
 *         0  - 无动作
 *         1  - 向右移动一次
 *        -1  - 向左移动一次
 *         2  - 快速向右（保持MENU_FAST_TRIGGER_TIME后）
 *        -2  - 快速向左（保持MENU_FAST_TRIGGER_TIME后）
 * @note 基于左摇杆X轴，划动触发一次，保持1秒后进入快速模式
 */
int8_t Joy_GetMenuControl(uint32_t delta_ms);

/**
 * @brief 获取校准后的中心值（用于调试）
 * @param left_x 左摇杆X轴中心值输出（可为NULL）
 * @param left_y 左摇杆Y轴中心值输出（可为NULL）
 * @param right_x 右摇杆X轴中心值输出（可为NULL）
 * @param right_y 右摇杆Y轴中心值输出（可为NULL）
 */
void Joy_GetCenterValues(uint16_t *left_x, uint16_t *left_y, uint16_t *right_x, uint16_t *right_y);

#ifdef __cplusplus
}
#endif

#endif /* __JOY_H__ */