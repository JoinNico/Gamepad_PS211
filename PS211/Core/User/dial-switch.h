#ifndef __DIAL_SWITCH_H
#define __DIAL_SWITCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

    // 虚拟设备ID定义
#define VGPIO_DEVICE_DIAL_LEFT    0xDEAD0001  // 左拨轮设备
#define VGPIO_DEVICE_DIAL_RIGHT   0xDEAD0002  // 右拨轮设备

// 虚拟GPIO结构（兼容您的格式）
typedef struct {
    void *port; // 设备标识
    void *pin; // 引脚号
    uint8_t state; // 电平状态
} vgpio_t;

// 初始化虚拟GPIO系统
void vgpio_init(void);

// 创建虚拟GPIO结构
vgpio_t vgpio_create(uint32_t device_id, uint8_t pin_num);

// 更新拨轮状态（需要定期调用）
void vgpio_update_dial_left(uint16_t adc_value);

void vgpio_update_dial_right(uint16_t adc_value);

// 获取虚拟GPIO状态（给lwbtn的回调）
uint8_t vgpio_get_state(void *port, void *pin);

/* ===== 拨轮配置 ===== */
#define DIAL_NUM_POSITIONS     3       // 三档拨轮
#define DIAL_SWITCH_COUNT      2       // 两个拨轮开关（左/右）

/* 拨轮ADC通道定义 - 与你的adc_process模块对应 */
#define DIAL_LEFT_CHANNEL      ADC_CH_DIAL_L       // 左拨轮ADC通道
#define DIAL_RIGHT_CHANNEL     ADC_CH_DIAL_R       // 右拨轮ADC通道

// 拨轮设备枚举
typedef enum {
    DIAL_LEFT = 0, // 左拨轮
    DIAL_RIGHT, // 右拨轮
    DIAL_COUNT
} dial_device_t;

// 初始化函数
void dial_switch_init(void);

// 处理函数（需要在主循环中定期调用）
void dial_switch_process(void);

// 获取当前档位
uint8_t dial_switch_get_position(dial_device_t device);

#ifdef __cplusplus
}
#endif

#endif /* __DIAL_SWITCH_H */