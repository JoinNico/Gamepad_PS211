#ifndef __ADC_PROCESS_H
#define __ADC_PROCESS_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdbool.h>

#define ADC_PROCESS_NUM_CHANNELS     11  // 总通道数
#define ADC_PROCESS_DMA_BUFFER_SIZE  (ADC_PROCESS_NUM_CHANNELS * 2) // DMA双缓冲大小

/* 通道索引定义 */
typedef enum {
    ADC_CH_JOY_LX = 0,    // ADC 通道1: 左摇杆 X 轴
    ADC_CH_JOY_LY,        // ADC 通道2: 左摇杆 Y 轴
    ADC_CH_JOY_RX,        // ADC 通道3: 右摇杆 X 轴
    ADC_CH_JOY_RY,        // ADC 通道4: 右摇杆 Y 轴
    ADC_CH_BAT_VOLT,      // ADC 通道6: 电池电压
    ADC_CH_WHEEL_L,       // ADC 通道8: 左滚轮
    ADC_CH_WHEEL_R,       // ADC 通道9: 右滚轮
    ADC_CH_DIAL_L,        // ADC 通道10: 左拨盘
    ADC_CH_DIAL_R,        // ADC 通道11: 右拨盘
    ADC_CH_5WAY_H,        // ADC 通道12: 左五向开关
    ADC_CH_5WAY_V,        // ADC 通道13: 右五向开关

    // 确保枚举最大值 = 10 与 ADC_PROCESS_NUM_CHANNELS - 1 一致
} ADC_ChannelIndex_t;

/* adc 模块主要数据结构 */
typedef struct {
    volatile uint16_t  raw_buffer[ADC_PROCESS_NUM_CHANNELS]; // 供外部读取的最终缓冲区
    bool               new_data_ready; // 新数据就绪标志
    float              battery_voltage;// 计算后的电池电压（单位：V）
} ADC_ProcessHandle_t;

/* 全局句柄（如果只需要一个ADC实例）*/
extern ADC_ProcessHandle_t hadcProc;

/* 公有函数声明 */
void ADC_PROCESS_Init(ADC_HandleTypeDef* hadc); // 初始化模块
bool ADC_PROCESS_IsDataReady(void);             // 检查新数据是否就绪
void ADC_PROCESS_ClearDataReadyFlag(void);      // 清除数据就绪标志
uint16_t ADC_PROCESS_GetRawValue(ADC_ChannelIndex_t ch); // 获取指定通道原始值
float ADC_PROCESS_GetBatteryVoltage(void);      // 获取计算后的电池电压（单位：V）
void adc_print_raw_buffer(const char *buffer_name,
                          uint16_t *buffer,
                          uint16_t size,
                          uint16_t adc_max,
                          float vref,
                          float divider_ratio);
void adc_print_raw_buffer_simple(const char *buffer_name,
                                 uint16_t *buffer,
                                 uint16_t size);
#ifdef __cplusplus
}
#endif

#endif /* __ADC_PROCESS_H */