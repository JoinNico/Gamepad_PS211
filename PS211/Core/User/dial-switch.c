#include "dial-switch.h"

#include "adc-process.h"

// 定义拨轮阈值（根据实际硬件调整）
static const uint16_t dial_thresholds[][2] = {
    // {min, max}
    {100, 999},     // 档位1
    {1000, 2999},  // 档位2
    {3000, 4095}   // 档位3
};

// 虚拟GPIO状态表
typedef struct {
    uint32_t device_id;  // 设备ID
    uint8_t pin_count;   // 引脚数量
    uint8_t states[3];   // 3个引脚的状态
} vgpio_device_t;

// 设备表
static vgpio_device_t vgpio_devices[] = {
    {VGPIO_DEVICE_DIAL_LEFT, 3, {0, 0, 0}},   // 左拨轮，3个档位
    {VGPIO_DEVICE_DIAL_RIGHT, 3, {0, 0, 0}},  // 右拨轮，3个档位
    {0, 0, {0, 0, 0}}  // 结束标志
};

// 当前档位
static uint8_t current_position_left = 0;
static uint8_t current_position_right = 0;

// 初始化
void vgpio_init(void) {
    // 初始化设备状态
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            vgpio_devices[i].states[j] = 0;
        }
    }
    current_position_left = 0;
    current_position_right = 0;
}

// 创建虚拟GPIO结构
vgpio_t vgpio_create(uint32_t device_id, uint8_t pin_num) {
    vgpio_t vgpio = {
        .port = (void*)(uintptr_t)device_id,
        .pin = (void*)(uintptr_t)pin_num,
        .state = 0
    };
    return vgpio;
}

// ADC值转换为档位
static uint8_t adc_to_position(uint16_t adc_value) {
    for (int i = 0; i < 3; i++) {
        if (adc_value >= dial_thresholds[i][0] &&
            adc_value <= dial_thresholds[i][1]) {
            return i + 1;  // 返回1,2,3
        }
    }
    return 0;  // 不在任何档位
}

// 更新拨轮状态
static void update_dial_state(uint32_t device_id, uint16_t adc_value, uint8_t* current_pos) {
    uint8_t new_pos = adc_to_position(adc_value);

    // 档位变化
    if (new_pos != *current_pos) {
        // 找到设备
        for (int i = 0; vgpio_devices[i].device_id != 0; i++) {
            if (vgpio_devices[i].device_id == device_id) {
                // 离开旧档位：对应GPIO置低
                if (*current_pos > 0 && *current_pos <= 3) {
                    vgpio_devices[i].states[*current_pos - 1] = 0;
                }

                // 进入新档位：对应GPIO置高
                if (new_pos > 0 && new_pos <= 3) {
                    vgpio_devices[i].states[new_pos - 1] = 1;
                }

                *current_pos = new_pos;
                break;
            }
        }
    }
}

// 更新左拨轮
void vgpio_update_dial_left(uint16_t adc_value) {
    update_dial_state(VGPIO_DEVICE_DIAL_LEFT, adc_value, &current_position_left);
}

// 更新右拨轮
void vgpio_update_dial_right(uint16_t adc_value) {
    update_dial_state(VGPIO_DEVICE_DIAL_RIGHT, adc_value, &current_position_right);
}

// 获取虚拟GPIO状态
uint8_t vgpio_get_state(void* port, void* pin) {
    uint32_t device_id = (uintptr_t)port;
    uint8_t pin_num = (uintptr_t)pin;

    // 检查参数有效性
    if (device_id == 0 || pin_num == 0 || pin_num > 3) {
        return 0;
    }

    // 查找设备
    for (int i = 0; vgpio_devices[i].device_id != 0; i++) {
        if (vgpio_devices[i].device_id == device_id) {
            if (pin_num <= vgpio_devices[i].pin_count) {
                return vgpio_devices[i].states[pin_num - 1];
            }
            break;
        }
    }

    return 0;
}



// 初始化拨轮开关
void dial_switch_init(void) {
    // 初始化虚拟GPIO
    vgpio_init();
}

// 处理拨轮开关状态
void dial_switch_process(void) {
    // 检查ADC数据是否就绪
    if (ADC_PROCESS_IsDataReady()) {
        // 获取ADC值
        uint16_t left_dial = ADC_PROCESS_GetRawValue(ADC_CH_DIAL_L);
        uint16_t right_dial = ADC_PROCESS_GetRawValue(ADC_CH_DIAL_R);

        // 更新虚拟GPIO状态
        vgpio_update_dial_left(left_dial);
        vgpio_update_dial_right(right_dial);

        // 清除标志
        ADC_PROCESS_ClearDataReadyFlag();
    }
}
