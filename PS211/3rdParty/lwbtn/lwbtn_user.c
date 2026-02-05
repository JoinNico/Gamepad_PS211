#include "lwbtn_opts.h"
#include "lwbtn.h"

#include "stm32f1xx_hal.h"
#include "../elog/elog.h"
#include "../WouoUI/WouoUI.h"
#include "../WouoUI/WouoUI_user.h"
#include "../../Core/User/dial-switch.h"
#include "../../Core/User/adc-process.h"

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

/* 左拨轮的3个档位按钮参数 */
lwbtn_argdata_port_pin_state_t dial_left_pos0 = {
    .port = (void*)VGPIO_DEVICE_DIAL_LEFT,
    .pin = (void*)1,  // 指向左拨轮实例
    .state = 0
};

lwbtn_argdata_port_pin_state_t dial_left_pos1 = {
    .port = (void*)VGPIO_DEVICE_DIAL_LEFT,
    .pin = (void*)2,
    .state = 0
};

lwbtn_argdata_port_pin_state_t dial_left_pos2 = {
    .port = (void*)VGPIO_DEVICE_DIAL_LEFT,
    .pin = (void*)3,
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
};

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

    uint32_t port_val = (uintptr_t)cfg->port;
    /* 判断按钮类型：GPIO或虚拟GPIO */
    if (port_val == VGPIO_DEVICE_DIAL_LEFT || port_val == VGPIO_DEVICE_DIAL_RIGHT) {
        /* 虚拟GPIO按钮：使用专用读取函数 */
        return vgpio_get_state(cfg->port, cfg->pin);

    } else {
        /* GPIO按钮：读取GPIO电平 */
        uint8_t pin_state = HAL_GPIO_ReadPin(cfg->port, (uint16_t)(uintptr_t)cfg->pin);
        return (pin_state == cfg->state) ? 1 : 0;
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
    uint8_t lwbtn_index = 0;

    switch (evt) {
        /* 按键按下事件 */
        case LWBTN_EVT_ONPRESS:

            break;

        /* 按键释放事件 */
        case LWBTN_EVT_ONRELEASE:

            break;

        /* 单击/双击事件 */
        case LWBTN_EVT_ONCLICK:
            lwbtn_index = btn_index + 1;
            if (btn->click.cnt == 2) {    /* 双击 */
                btn->click.cnt = 0;
                switch (lwbtn_index) {
                    case 1:  // 按键1按下
                        log_i("Button 1 doubled pressed");
                        // 处理按键1按下的操作
                        WOUOUI_MSG_QUE_SEND(msg_click);
                        break;
                    case 6:  // 按键2按下
                        log_i("Button 2 doubled pressed");
                        // 处理按键2按下的操作
                        WOUOUI_MSG_QUE_SEND(msg_return);
                        break;
                }
            } else { /* 单击 */
                switch (lwbtn_index) {
                    case 5:  // 按键1按下
                        log_i("Button 1 pressed");
                        // 处理按键1按下的操作
                        WOUOUI_MSG_QUE_SEND(msg_left);
                        break;
                    case 6:  // 按键2按下
                        log_i("Button 2 pressed");
                        // 处理按键2按下的操作
                        WOUOUI_MSG_QUE_SEND(msg_click);
                        break;
                    case 7:  // 按键2按下
                        log_i("Button 2 pressed");
                        // 处理按键2按下的操作
                        WOUOUI_MSG_QUE_SEND(msg_right);
                        break;
                }
            }
            break;

        /* 长按事件 */
        case LWBTN_EVT_KEEPALIVE:
            lwbtn_index = btn_index + 1;
            // log_i("Press and hold key %d\n", lwbtn_index);
            break;
    }
}

/**
 * @brief 初始化按钮管理器（包括拨轮）
 */
void button_init(void)
{
    vgpio_init();
    /* 初始化LwBTN管理器 */
    lwbtn_init_ex(NULL, buttons, BUTTON_COUNT, get_button_state, button_event_handler);

    log_i("Button manager initialized with %d buttons\n", BUTTON_COUNT);
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

        dial_switch_process();
        /* 处理LwBTN按钮 */
        lwbtn_process(current_time);
    }
}