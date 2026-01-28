#ifndef __DIAL_SWITCH_H
#define __DIAL_SWITCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "../../3rdParty/lwbtn.h"  // 添加LwBTN支持

/* ===== 拨轮配置 ===== */
#define DIAL_NUM_POSITIONS     3       // 三档拨轮
#define DIAL_SWITCH_COUNT      2       // 两个拨轮开关（左/右）

/* 拨轮ADC通道定义 - 与你的adc_process模块对应 */
#define DIAL_LEFT_CHANNEL      4       // 左拨轮ADC通道
#define DIAL_RIGHT_CHANNEL     5       // 右拨轮ADC通道

/* ADC电压阈值定义 (12位ADC, 0-4095) */
#define DIAL_THRESHOLDS_CONFIG \
    {0,    1350,   0},     /* 档位0: 通常对应最左/下位置 */ \
    {1351, 2750,   1},     /* 档位1: 中间位置 */ \
    {2751, 4095,   2}      /* 档位2: 最右/上位置 */

/* 防抖配置 */
#define DIAL_DEBOUNCE_MS       20      // 防抖时间(毫秒)
#define DIAL_HYSTERESIS        50      // 滞后区域，防止临界值抖动

/* ===== 结构体定义 ===== */

/* 拨轮档位电压阈值结构 */
typedef struct {
    uint16_t min_adc;      // 最小ADC值
    uint16_t max_adc;      // 最大ADC值
    uint8_t  position;     // 对应的档位(0,1,2...)
} Dial_Threshold_t;

/* 拨轮开关实例句柄 */
typedef struct {
    /* 配置参数 */
    const Dial_Threshold_t *thresholds;    // 阈值表指针
    uint8_t threshold_count;               // 阈值数量

    /* 状态变量 */
    uint8_t  current_position;             // 当前档位
    uint8_t  stable_position;              // 稳定后的档位(防抖后)
    uint8_t  last_stable_position;         // 上一次稳定档位
    uint16_t last_raw_adc;                 // 上一次ADC原始值

    /* LwBTN相关 */
    lwbtn_btn_t *lwbtn_instance;           // 对应的LwBTN按钮实例
    lwbtn_argdata_port_pin_state_t lwbtn_arg; // LwBTN参数

    /* 时间戳 */
    uint32_t position_change_time;         // 档位变化时间戳
    uint32_t last_update_time;             // 上次更新时间戳

    /* 防抖计数器 */
    uint32_t debounce_counter;             // 防抖计数器(毫秒)

    /* 统计信息 */
    uint32_t position_changes;             // 档位切换次数统计

    /* 调试标志 */
    bool debug_enable;
} Dial_Handle_t;

/* ===== LwBTN相关定义 ===== */

/* 拨轮LwBTN按钮索引（在总按钮数组中的位置） */
#define DIAL_LWBTN_START_INDEX 4  // GPIO按钮后开始（你的有4个GPIO按钮）
#define DIAL_LEFT_INDEX        0  // 左拨轮在拨轮数组中的索引
#define DIAL_RIGHT_INDEX       1  // 右拨轮在拨轮数组中的索引

/* 拨轮LwBTN回调事件 */
typedef void (*Dial_LwBtnCallback_t)(uint8_t dial_index, uint8_t position, uint8_t event);

/* ===== 公有函数声明 ===== */

/* 拨轮核心功能 */
bool Dial_Init(Dial_Handle_t *hdial,
               const Dial_Threshold_t *thresholds,
               uint8_t threshold_count,
               uint16_t init_adc_value,
               lwbtn_btn_t *lwbtn_ptr);    // 新增LwBTN实例参数
void Dial_InitAll(void);
bool Dial_Update(Dial_Handle_t *hdial, uint16_t raw_adc, uint32_t current_time_ms);
uint8_t Dial_GetPosition(const Dial_Handle_t *hdial);
uint16_t Dial_GetRawADC(const Dial_Handle_t *hdial);

/* LwBTN集成功能 */
void Dial_RegisterLwBtnCallback(Dial_LwBtnCallback_t callback);
uint8_t Dial_GetLwBtnState(uint8_t dial_index, uint8_t position);
lwbtn_btn_t* Dial_GetLwBtnInstances(void);
uint8_t Dial_GetLwBtnCount(void);

uint8_t Dial_LwBtn_GetState(lwbtn_t *lwobj, lwbtn_btn_t *btn);

/* 调试功能 */
void Dial_SetDebug(Dial_Handle_t *hdial, bool enable);

/* ===== 全局实例 ===== */
extern Dial_Handle_t dial_switches[DIAL_SWITCH_COUNT];
extern const Dial_Threshold_t DIAL_DEFAULT_THRESHOLDS[DIAL_NUM_POSITIONS];

#ifdef __cplusplus
}
#endif

#endif /* __DIAL_SWITCH_H */