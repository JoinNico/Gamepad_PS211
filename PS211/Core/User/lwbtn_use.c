#include "../../3rdParty/lwbtn_opts.h"
#include "../../3rdParty/lwbtn.h"
#include "stm32f1xx_hal.h"
#include "../../3rdParty/elog.h"

/**
 * @brief 全局变量，记录当前触发的按键编号
 *
 * - 0: 无按键
 * - 1: KEY0
 * - 2: KEY1
 * - 3: KEY2
 * - 4: KEY_UP
 * 必须要 lwbtn_keys = btn_index + 1; 以摆脱为0时的按键状态
 */
uint8_t lwbtn_keys;


/* 为每个按键定义GPIO参数 */
lwbtn_argdata_port_pin_state_t joy_zl_gpio = {
    .port = JOY_ZL_GPIO_Port,  /* KEY0的GPIO端口 */
    .pin = (void*)JOY_ZL_Pin,         /* KEY0的GPIO引脚 */
    .state = 0               /* 低电平触发 */
};

lwbtn_argdata_port_pin_state_t joy_zr_gpio = {
    .port = JOY_ZR_GPIO_Port, /* KEY_UP的GPIO端口 */
    .pin = (void*)JOY_ZR_Pin,        /* KEY_UP的GPIO引脚 */
    .state = 0                /* 高电平触发 */
};

lwbtn_argdata_port_pin_state_t key1_gpio = {
    .port = KEY1_GPIO_Port,  /* KEY1的GPIO端口 */
    .pin = (void*)KEY1_Pin,         /* KEY1的GPIO引脚 */
    .state = 0               /* 低电平触发 */
};

lwbtn_argdata_port_pin_state_t key2_gpio = {
    .port = KEY2_GPIO_Port,  /* KEY2的GPIO端口 */
    .pin = (void*)KEY2_Pin,         /* KEY2的GPIO引脚 */
    .state = 0               /* 低电平触发 */
};

/* 定义按钮数组，每个按钮绑定对应的GPIO参数 */
lwbtn_btn_t buttons[] = {
    { .arg = &joy_zl_gpio },    /* JOY_ZL */
    { .arg = &joy_zr_gpio },    /* JOY_ZR */
    { .arg = &key1_gpio },      /* KEY1 */
    { .arg = &key2_gpio },      /* KEY2 */
};


/**
 * @brief 获取按钮的当前状态
 *
 * 通过读取GPIO引脚的电平，判断按钮是否处于激活状态。
 *
 * @param lwobj: LwBTN实例（未使用）
 * @param btn: 按钮实例，包含GPIO参数
 * @return uint8_t: 1表示按钮激活，0表示按钮未激活
 */
uint8_t get_button_state(lwbtn_t* lwobj, lwbtn_btn_t* btn)
{
    lwbtn_argdata_port_pin_state_t* cfg = btn->arg;
    uint8_t pin_state = HAL_GPIO_ReadPin(cfg->port, (uint16_t)(uintptr_t)cfg->pin);
    return (pin_state == cfg->state) ? 1 : 0; // 返回激活状态
}

/**
 * @brief 按钮事件处理函数
 *
 * 根据LwBTN库触发的事件类型，处理按钮的按下、释放、单击、双击和长按事件。
 * * 每一个事件都会触发一个回调 *
 *
 * @param lwobj: LwBTN实例
 * @param btn: 触发事件的按钮实例
 * @param evt: 事件类型（按下、释放、单击、长按等）
 */
void button_event_handler(lwbtn_t* lwobj, lwbtn_btn_t* btn, lwbtn_evt_t evt)
{
    // 根据按钮索引识别具体按键
    uint8_t btn_index = (btn - lwobj->btns);
    switch (evt) {
        /* 按键按下事件 */
        case LWBTN_EVT_ONPRESS:
//            lwbtn_keys = btn_index + 1; // 记录按键编号
            break;
        /* 按键释放事件 */
        case LWBTN_EVT_ONRELEASE:
//            lwbtn_keys = btn_index + 1; // 记录按键编号
            break;
        /* 单击/双击事件 */
        case LWBTN_EVT_ONCLICK:
            if (btn->click.cnt == 2) {	/* 双击 */
                lwbtn_keys = btn_index + 1; // 记录按键编号
                btn->click.cnt = 0; 				// 重置计数器
                log_i("Double-click key %d\r\n",lwbtn_keys);
            } else { /* 单击 */
                lwbtn_keys = btn_index + 1; // 记录按键编号
                log_i("Click the key %d\r\n",lwbtn_keys);
            }
            break;
        /* 长按事件 */
        case LWBTN_EVT_KEEPALIVE:
            lwbtn_keys = btn_index + 1; // 记录按键编号
            log_i("Press and hold key %d\n", lwbtn_keys);
            break;
    }
}

/**
 * @brief 初始化按钮管理器
 *
 * 使用LwBTN库初始化按钮管理器，绑定按钮数组、状态读取函数和事件处理函数。
 */
void button_init(void)
{
    lwbtn_init_ex(NULL, buttons, BUTTON_COUNT, get_button_state, button_event_handler);
}


/**
 * @brief 处理按键状态
 *
 * 定期调用此函数以处理按键状态，触发事件回调。
 */
void get_btn()
{
    uint32_t tick = 0;
    tick = HAL_GetTick(); // 获取当前系统时间
    lwbtn_process(tick);  // 处理按键状态
}