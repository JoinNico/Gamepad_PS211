#include "dial-switch.h"
#include <stdio.h>  // 仅用于调试打印

/* ===== 全局变量 ===== */
Dial_Handle_t dial_switches[DIAL_SWITCH_COUNT];
static Dial_LwBtnCallback_t dial_lwbtn_callback = NULL;

/* LwBTN按钮实例数组（为每个拨轮的每个档位创建实例）*/
static lwbtn_btn_t dial_lwbtn_instances[DIAL_SWITCH_COUNT * DIAL_NUM_POSITIONS];

/* 默认阈值表 */
const Dial_Threshold_t DIAL_DEFAULT_THRESHOLDS[DIAL_NUM_POSITIONS] = {
    DIAL_THRESHOLDS_CONFIG
};

/* ===== 私有函数声明 ===== */
static uint8_t Dial_ADCtoPosition(const Dial_Handle_t *hdial, uint16_t adc_value);
static bool Dial_IsInHysteresis(const Dial_Handle_t *hdial, uint16_t new_adc, uint16_t old_adc);
static void Dial_PrintDebug(const Dial_Handle_t *hdial, const char *message, uint16_t adc_value);
static void Dial_UpdateLwBtnState(Dial_Handle_t *hdial, uint8_t old_position, uint8_t new_position);
static uint8_t Dial_GetLwBtnIndex(uint8_t dial_index, uint8_t position);

/* ===== 公有函数实现 ===== */

/**
  * @brief  初始化拨轮开关实例
  */
bool Dial_Init(Dial_Handle_t *hdial,
               const Dial_Threshold_t *thresholds,
               uint8_t threshold_count,
               uint16_t init_adc_value,
               lwbtn_btn_t *lwbtn_ptr)
{
    if (hdial == NULL || thresholds == NULL || threshold_count == 0) {
        return false;
    }

    /* 初始化配置 */
    hdial->thresholds = thresholds;
    hdial->threshold_count = threshold_count;

    /* 初始状态 */
    hdial->current_position = Dial_ADCtoPosition(hdial, init_adc_value);
    hdial->stable_position = hdial->current_position;
    hdial->last_stable_position = hdial->stable_position;
    hdial->last_raw_adc = init_adc_value;

    /* LwBTN设置 */
    hdial->lwbtn_instance = lwbtn_ptr;
    if (lwbtn_ptr != NULL) {
        /* 初始化LwBTN参数 */
        hdial->lwbtn_arg.port = NULL;  // 不使用GPIO
        hdial->lwbtn_arg.pin = (void*)hdial;  // 指向拨轮实例作为标识
        hdial->lwbtn_arg.state = 0;

        /* 设置LwBTN按钮参数 */
        lwbtn_ptr->arg = &hdial->lwbtn_arg;
    }

    /* 初始化时间戳和计数器 */
    hdial->position_change_time = 0;
    hdial->last_update_time = 0;
    hdial->debounce_counter = 0;

    /* 统计信息 */
    hdial->position_changes = 0;

    /* 默认关闭调试 */
    hdial->debug_enable = false;

    Dial_PrintDebug(hdial, "Dial initialized", init_adc_value);
    return true;
}

/**
  * @brief  初始化所有拨轮开关
  */
void Dial_InitAll(void)
{
    /* 初始化左拨轮 */
    Dial_Init(&dial_switches[DIAL_LEFT_INDEX],
              DIAL_DEFAULT_THRESHOLDS,
              DIAL_NUM_POSITIONS,
              2048,  // 初始ADC值设为中间值
              NULL); // LwBTN实例稍后设置

    /* 初始化右拨轮 */
    Dial_Init(&dial_switches[DIAL_RIGHT_INDEX],
              DIAL_DEFAULT_THRESHOLDS,
              DIAL_NUM_POSITIONS,
              2048,  // 初始ADC值设为中间值
              NULL); // LwBTN实例稍后设置
}

/**
  * @brief  更新拨轮状态
  */
bool Dial_Update(Dial_Handle_t *hdial, uint16_t raw_adc, uint32_t current_time_ms)
{
    if (hdial == NULL) {
        return false;
    }

    bool position_changed = false;

    /* 计算时间增量 */
    uint32_t delta_time = (hdial->last_update_time == 0) ? 0 :
                          (current_time_ms - hdial->last_update_time);
    hdial->last_update_time = current_time_ms;

    /* 检查ADC值是否在滞后区域内 */
    if (Dial_IsInHysteresis(hdial, raw_adc, hdial->last_raw_adc)) {
        hdial->last_raw_adc = raw_adc;
        return false;
    }

    /* 将ADC值转换为档位 */
    uint8_t new_position = Dial_ADCtoPosition(hdial, raw_adc);
    hdial->last_raw_adc = raw_adc;

    /* 检查档位是否变化 */
    if (new_position != hdial->current_position) {
        uint8_t old_position = hdial->current_position;
        hdial->current_position = new_position;
        hdial->debounce_counter = 0;
        hdial->position_change_time = current_time_ms;

        Dial_PrintDebug(hdial, "Position raw change detected", raw_adc);
    }

    /* 防抖处理 */
    if (hdial->current_position != hdial->stable_position) {
        hdial->debounce_counter += delta_time;

        if (hdial->debounce_counter >= DIAL_DEBOUNCE_MS) {
            /* 档位稳定确认 */
            uint8_t old_position = hdial->stable_position;
            hdial->stable_position = hdial->current_position;
            hdial->last_stable_position = old_position;
            hdial->position_changes++;
            position_changed = true;

            Dial_PrintDebug(hdial, "Position stabilized", raw_adc);

            /* 更新LwBTN状态 */
            Dial_UpdateLwBtnState(hdial, old_position, hdial->stable_position);

            /* 触发回调函数 */
            if (dial_lwbtn_callback != NULL) {
                // 确定拨轮索引
                uint8_t dial_index = (hdial == &dial_switches[0]) ? 0 : 1;
                dial_lwbtn_callback(dial_index, hdial->stable_position, 1); // 1=位置变化事件
            }
        }
    }

    return position_changed;
}

/**
  * @brief  更新所有拨轮状态
  */
void Dial_UpdateAll(uint16_t left_adc, uint16_t right_adc, uint32_t current_time_ms)
{
    Dial_Update(&dial_switches[DIAL_LEFT_INDEX], left_adc, current_time_ms);
    Dial_Update(&dial_switches[DIAL_RIGHT_INDEX], right_adc, current_time_ms);
}

/**
  * @brief  更新LwBTN按钮状态
  */
static void Dial_UpdateLwBtnState(Dial_Handle_t *hdial, uint8_t old_position, uint8_t new_position)
{
    /* 如果拨轮有对应的LwBTN实例数组 */
    if (hdial->lwbtn_instance != NULL) {
        // 这里可以实现将拨轮档位映射到多个LwBTN按钮
        // 例如：每个档位对应一个独立的LwBTN按钮
    }
}

/**
  * @brief  获取LwBTN按钮状态（用于自定义读取函数）
  */
uint8_t Dial_GetLwBtnState(uint8_t dial_index, uint8_t position)
{
    if (dial_index >= DIAL_SWITCH_COUNT) {
        return 0;
    }

    Dial_Handle_t *hdial = &dial_switches[dial_index];

    /* 如果指定了特定档位，检查是否匹配 */
    if (position < 0xFF) {
        return (hdial->stable_position == position) ? 1 : 0;
    }

    /* 否则返回当前档位作为状态（0-2） */
    return hdial->stable_position;
}

/**
  * @brief  注册LwBTN回调函数
  */
void Dial_RegisterLwBtnCallback(Dial_LwBtnCallback_t callback)
{
    dial_lwbtn_callback = callback;
}

/**
  * @brief  获取LwBTN按钮实例数组
  */
lwbtn_btn_t* Dial_GetLwBtnInstances(void)
{
    return dial_lwbtn_instances;
}

/**
  * @brief  获取LwBTN按钮数量
  */
uint8_t Dial_GetLwBtnCount(void)
{
    return DIAL_SWITCH_COUNT * DIAL_NUM_POSITIONS;
}

/**
  * @brief  ADC值转换为档位
  */
static uint8_t Dial_ADCtoPosition(const Dial_Handle_t *hdial, uint16_t adc_value)
{
    for (uint8_t i = 0; i < hdial->threshold_count; i++) {
        if (adc_value >= hdial->thresholds[i].min_adc &&
            adc_value <= hdial->thresholds[i].max_adc) {
            return hdial->thresholds[i].position;
        }
    }

    /* 未匹配任何档位，返回无效值 */
    return 0xFF;
}

/**
  * @brief  滞后区域检查
  */
static bool Dial_IsInHysteresis(const Dial_Handle_t *hdial, uint16_t new_adc, uint16_t old_adc)
{
    int32_t diff = (int32_t)new_adc - (int32_t)old_adc;
    if (diff < 0) diff = -diff;

    return (diff <= DIAL_HYSTERESIS);
}

/**
  * @brief  调试信息输出
  */
static void Dial_PrintDebug(const Dial_Handle_t *hdial, const char *message, uint16_t adc_value)
{
    if (hdial->debug_enable) {
        printf("[DIAL] %s | ADC: %u | Pos: %d | Stable: %d\n",
               message, adc_value, hdial->current_position, hdial->stable_position);
    }
}

/* ===== 其他公有函数实现 ===== */

uint8_t Dial_GetPosition(const Dial_Handle_t *hdial)
{
    return (hdial ? hdial->stable_position : 0xFF);
}

uint16_t Dial_GetRawADC(const Dial_Handle_t *hdial)
{
    return (hdial ? hdial->last_raw_adc : 0);
}

void Dial_SetDebug(Dial_Handle_t *hdial, bool enable)
{
    if (hdial) hdial->debug_enable = enable;
}

/**
  * @brief  拨轮LwBTN状态读取函数（集成到LwBTN中）
  */
uint8_t Dial_LwBtn_GetState(lwbtn_t* lwobj, lwbtn_btn_t* btn)
{
    /* 从按钮参数中获取拨轮信息 */
    lwbtn_argdata_port_pin_state_t* cfg = btn->arg;
    if (cfg == NULL || cfg->pin == NULL) {
        return 0;
    }

    /* pin指针指向Dial_Handle_t实例 */
    Dial_Handle_t* hdial = (Dial_Handle_t*)cfg->pin;

    /* 根据按钮索引确定是哪个档位 */
    uint8_t btn_index = (btn - lwobj->btns) - DIAL_LWBTN_START_INDEX;
    uint8_t dial_index = btn_index / DIAL_NUM_POSITIONS;
    uint8_t position = btn_index % DIAL_NUM_POSITIONS;

    /* 检查该按钮对应的档位是否激活 */
    if (dial_index < DIAL_SWITCH_COUNT) {
        Dial_Handle_t* target_dial = &dial_switches[dial_index];
        return (target_dial->stable_position == position) ? 1 : 0;
    }

    return 0;
}