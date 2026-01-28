#include "../../3rdParty/lwbtn_opts.h"
#include "../../3rdParty/lwbtn.h"
#include "stm32f1xx_hal.h"
#include "../../3rdParty/elog.h"
#include "dial-switch.h"  // 添加拨轮开关支持

/**
 * @brief 全局变量，记录当前触发的按键编号
 *
 * - 0: 无按键
 * - 1-3: GPIO按钮
 * - 4-9: 拨轮按钮（左拨轮3个档位 + 右拨轮3个档位）
 */
uint8_t lwbtn_keys;

/* ===== GPIO按钮定义 ===== */
lwbtn_argdata_port_pin_state_t joy_zl_gpio = {
    .port = JOY_ZL_GPIO_Port,
    .pin = (void*)JOY_ZL_Pin,
    .state = 0
};

lwbtn_argdata_port_pin_state_t joy_zr_gpio = {
    .port = JOY_ZR_GPIO_Port,
    .pin = (void*)JOY_ZR_Pin,
    .state = 0
};

lwbtn_argdata_port_pin_state_t key1_gpio = {
    .port = KEY1_GPIO_Port,
    .pin = (void*)KEY1_Pin,
    .state = 0
};

lwbtn_argdata_port_pin_state_t key2_gpio = {
    .port = KEY2_GPIO_Port,
    .pin = (void*)KEY2_Pin,
    .state = 0
};

/* ===== 拨轮按钮定义 ===== */
/* 注意：拨轮按钮使用自定义参数，pin字段指向拨轮实例 */

/* 左拨轮的3个档位按钮参数 */
lwbtn_argdata_port_pin_state_t dial_left_pos0 = {
    .port = NULL,
    .pin = (void*)&dial_switches[DIAL_LEFT_INDEX],  // 指向左拨轮实例
    .state = 0
};

lwbtn_argdata_port_pin_state_t dial_left_pos1 = {
    .port = NULL,
    .pin = (void*)&dial_switches[DIAL_LEFT_INDEX],
    .state = 0
};

lwbtn_argdata_port_pin_state_t dial_left_pos2 = {
    .port = NULL,
    .pin = (void*)&dial_switches[DIAL_LEFT_INDEX],
    .state = 0
};

/* 右拨轮的3个档位按钮参数 */
lwbtn_argdata_port_pin_state_t dial_right_pos0 = {
    .port = NULL,
    .pin = (void*)&dial_switches[DIAL_RIGHT_INDEX], // 指向右拨轮实例
    .state = 0
};

lwbtn_argdata_port_pin_state_t dial_right_pos1 = {
    .port = NULL,
    .pin = (void*)&dial_switches[DIAL_RIGHT_INDEX],
    .state = 0
};

lwbtn_argdata_port_pin_state_t dial_right_pos2 = {
    .port = NULL,
    .pin = (void*)&dial_switches[DIAL_RIGHT_INDEX],
    .state = 0
};

/* ===== 按钮数组 ===== */
lwbtn_btn_t buttons[] = {
    /* GPIO按钮 (4个) */
    { .arg = &joy_zl_gpio },    /* JOY_ZL - 索引 0 */
    { .arg = &joy_zr_gpio },    /* JOY_ZR - 索引 1 */
    { .arg = &key1_gpio },      /* KEY1   - 索引 2 */
    { .arg = &key2_gpio },      /* KEY2   - 索引 3 */

    /* 拨轮按钮 (6个) */
    { .arg = &dial_left_pos0 },  /* 左拨轮档位0 - 索引 4 */
    { .arg = &dial_left_pos1 },  /* 左拨轮档位1 - 索引 5 */
    { .arg = &dial_left_pos2 },  /* 左拨轮档位2 - 索引 6 */
    { .arg = &dial_right_pos0 }, /* 右拨轮档位0 - 索引 7 */
    { .arg = &dial_right_pos1 }, /* 右拨轮档位1 - 索引 8 */
    { .arg = &dial_right_pos2 }, /* 右拨轮档位2 - 索引 9 */
};

#define TOTAL_BUTTON_COUNT (sizeof(buttons) / sizeof(buttons[0]))

/**
 * @brief 统一的按钮状态读取函数
 *
 * 支持GPIO按钮和拨轮按钮
 */
uint8_t get_button_state(lwbtn_t* lwobj, lwbtn_btn_t* btn)
{
    if (btn == NULL || btn->arg == NULL) {
        return 0;
    }

    lwbtn_argdata_port_pin_state_t* cfg = btn->arg;

    /* 判断按钮类型：GPIO或拨轮 */
    if (cfg->port != NULL) {
        /* GPIO按钮：读取GPIO电平 */
        uint8_t pin_state = HAL_GPIO_ReadPin(cfg->port, (uint16_t)(uintptr_t)cfg->pin);
        return (pin_state == cfg->state) ? 1 : 0;
    } else {
        /* 拨轮按钮：使用拨轮专用读取函数 */
        return Dial_LwBtn_GetState(lwobj, btn);
    }
}

/**
 * @brief 按钮事件处理函数
 *
 * 处理所有按钮（GPIO和拨轮）的事件
 */
void button_event_handler(lwbtn_t* lwobj, lwbtn_btn_t* btn, lwbtn_evt_t evt)
{
    uint8_t btn_index = (btn - lwobj->btns);

    switch (evt) {
        /* 按键按下事件 */
        case LWBTN_EVT_ONPRESS:
            // lwbtn_keys = btn_index + 1;
            break;

        /* 按键释放事件 */
        case LWBTN_EVT_ONRELEASE:
            // lwbtn_keys = btn_index + 1;
            break;

        /* 单击/双击事件 */
        case LWBTN_EVT_ONCLICK:
            if (btn->click.cnt == 2) {    /* 双击 */
                lwbtn_keys = btn_index + 1;
                btn->click.cnt = 0;
                log_i("Double-click key %d\r\n", lwbtn_keys);
            } else { /* 单击 */
                lwbtn_keys = btn_index + 1;
                log_i("Click the key %d\r\n", lwbtn_keys);
            }
            break;

        /* 长按事件 */
        case LWBTN_EVT_KEEPALIVE:
            lwbtn_keys = btn_index + 1;
            log_i("Press and hold key %d\n", lwbtn_keys);
            break;
    }

    /* 拨轮按钮特殊处理 */
    if (btn_index >= DIAL_LWBTN_START_INDEX) {
        uint8_t dial_index = (btn_index - DIAL_LWBTN_START_INDEX) / DIAL_NUM_POSITIONS;
        uint8_t position = (btn_index - DIAL_LWBTN_START_INDEX) % DIAL_NUM_POSITIONS;

        log_i("Dial %d position %d event %d\n", dial_index, position, evt);
    }
}

/**
 * @brief 拨轮LwBTN回调函数示例
 */
static void dial_lwbtn_callback(uint8_t dial_index, uint8_t position, uint8_t event)
{
    log_i("Dial %d changed to position %d (event %d)\n", dial_index, position, event);

    /* 根据拨轮位置执行不同操作 */
    switch (dial_index) {
        case DIAL_LEFT_INDEX:
            // 左拨轮操作
            break;
        case DIAL_RIGHT_INDEX:
            // 右拨轮操作
            break;
    }
}

/**
 * @brief 初始化按钮管理器（包括拨轮）
 */
void button_init(void)
{
    /* 初始化拨轮开关 */
    Dial_InitAll();
    Dial_RegisterLwBtnCallback(dial_lwbtn_callback);

    /* 初始化LwBTN管理器 */
    lwbtn_init_ex(NULL, buttons, TOTAL_BUTTON_COUNT, get_button_state, button_event_handler);

    log_i("Button manager initialized with %d buttons\n", TOTAL_BUTTON_COUNT);
}

/**
 * @brief 更新所有输入状态（包括拨轮）
 */
void get_btn(void)
{
    static uint32_t last_update = 0;
    uint32_t current_time = HAL_GetTick();

    /* 每10ms更新一次 */
    if (current_time - last_update >= 10) {
        last_update = current_time;

        /* 更新拨轮状态（需要从ADC获取值） */
        // 注意：这里需要你从adc_process模块获取拨轮ADC值
        // uint16_t left_dial_adc = ADC_PROCESS_GetRawValue(DIAL_LEFT_CHANNEL);
        // uint16_t right_dial_adc = ADC_PROCESS_GetRawValue(DIAL_RIGHT_CHANNEL);
        // Dial_UpdateAll(left_dial_adc, right_dial_adc, current_time);

        /* 处理LwBTN按钮 */
        lwbtn_process(current_time);
    }
}

/**
 * @brief 获取拨轮状态信息（调试用）
 */
void get_dial_status(void)
{
    for (int i = 0; i < DIAL_SWITCH_COUNT; i++) {
        uint8_t pos = Dial_GetPosition(&dial_switches[i]);
        uint16_t adc = Dial_GetRawADC(&dial_switches[i]);
        log_i("Dial %d: Position=%d, ADC=%u\n", i, pos, adc);
    }
}