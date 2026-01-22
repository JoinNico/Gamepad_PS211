#include "joystick.h"

#include <stdlib.h>
#include <math.h>
#include "adc.h"
#include "tim.h"
#include "debug.h"


/* 私有变量 */
volatile static uint16_t adc_buffer[4] = { 0 };  // ADC DMA缓冲区，4个通道
static JoyStick_t leftJoy;              // 左摇杆
static JoyStick_t rightJoy;             // 右摇杆

/* 校准中心值 */
static uint16_t left_x_center = 2048;
static uint16_t left_y_center = 2048;
static uint16_t right_x_center = 2048;
static uint16_t right_y_center = 2048;

/* 移动平均滤波缓冲区 */
static uint16_t filter_lx[WIN_SIZE], filter_ly[WIN_SIZE],
                filter_rx[WIN_SIZE], filter_ry[WIN_SIZE];

/* 菜单控制相关 */
static uint32_t menu_hold_timer = 0;
static int8_t last_menu_direction = 0;  // -1:左, 0:中间, 1:右
static uint8_t menu_fast_mode = 0;

/**
 * @brief 中心死区处理
 * @param value 相对中心的偏移值
 * @param center_deadzone 中心死区大小
 * @return 处理后的值
 */
static int16_t ApplyCenterDeadzone(int16_t value, int16_t center_deadzone)
{
    if (value > center_deadzone) {
        return value - center_deadzone;
    } else if (value < -center_deadzone) {
        return value + center_deadzone;
    }
    return 0;
}

/**
 * @brief 圆周死区处理并归一化到[0, 1]
 * @param x X轴值（已过中心死区）
 * @param y Y轴值（已过中心死区）
 * @param max_range 最大有效范围
 * @param edge_deadzone 圆周死区大小
 * @param out_x 输出归一化X值
 * @param out_y 输出归一化Y值
 */
static void ApplyRadialDeadzoneAndNormalize(int16_t x, int16_t y,
                                            int16_t max_range,
                                            int16_t edge_deadzone,
                                            float *out_x, float *out_y)
{
    // 计算距离中心的距离
    float magnitude = sqrtf((float)(x * x + y * y));

    if (magnitude < 0.1f) {
        // 太接近中心，直接归零
        *out_x = 0.0f;
        *out_y = 0.0f;
        return;
    }

    // 计算方向
    float nx = (float)x / magnitude;
    float ny = (float)y / magnitude;

    // 有效范围（扣除边缘死区）
    float effective_max = (float)(max_range - edge_deadzone);

    // 应用圆周死区并归一化
    float normalized_magnitude;
    if (magnitude > (float)(max_range - edge_deadzone)) {
        // 在边缘死区内，强制为最大值
        normalized_magnitude = 1.0f;
    } else {
        // 线性映射到[0, 1]
        normalized_magnitude = magnitude / effective_max;
        if (normalized_magnitude > 1.0f) {
            normalized_magnitude = 1.0f;
        }
    }

    // 输出归一化后的坐标
    *out_x = nx * normalized_magnitude;
    *out_y = ny * normalized_magnitude;
}

/**
 * @brief 对数曲线映射（用于油门控制）
 * @param input 输入值 0 到 1
 * @param is_negative 是否为负方向
 * @return 对数映射后的值 0 到 1
 */
static float LogCurve(float input, uint8_t is_negative)
{
    if (input < 0.001f) return 0.0f;

    // 对数曲线: y = (e^(x*2) - 1) / (e^2 - 1)
    // 在小输入时更灵敏，大输入时平滑
    float log_value = (expf(input * 2.0f) - 1.0f) / (expf(2.0f) - 1.0f);

    return is_negative ? -log_value : log_value;
}

/**
 * @brief 摇杆初始化
 * @note 必须在使用前调用，会启动ADC DMA采集
 *       自动检测摇杆中心位置并校准（允许±CENTER_CALIBRATION_TOL偏差）
 *       如果检测到的中心值偏离ADC_CENTER超过容差，则使用默认值
 *       初始化滤波缓冲区为中心值
 */
void Joy_Init(void)
{
    // ADC 采样校准
    HAL_ADCEx_Calibration_Start(&hadc1);

    // 启动ADC DMA
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, 4) != HAL_OK) {
        Error_Handler();
    }

    // 启动 TIM8 以开始产生 100Hz TRGO 信号 -> 10ms 一次 ADC 转换
    HAL_TIM_Base_Start(&htim8);

    // 确保初始化校准数据干净
    HAL_Delay(5);

    // 多次采样求平均作为中心值，提高校准精度
#define INIT_SAMPLE_COUNT 10
    uint32_t lx_sum = 0, ly_sum = 0, rx_sum = 0, ry_sum = 0;

    for (int i = 0; i < INIT_SAMPLE_COUNT; i++) {
        HAL_Delay(15); // 间隔采样，此时OS未启动，无法使用osDelay();
        lx_sum += adc_buffer[0];
        ly_sum += adc_buffer[1];
        rx_sum += adc_buffer[2];
        ry_sum += adc_buffer[3];
    }

    // 计算平均值
    uint16_t left_x_raw  = lx_sum / INIT_SAMPLE_COUNT;
    uint16_t left_y_raw  = ly_sum / INIT_SAMPLE_COUNT;
    uint16_t right_x_raw = rx_sum / INIT_SAMPLE_COUNT;
    uint16_t right_y_raw = ry_sum / INIT_SAMPLE_COUNT;

    // 校准摇杆中心值
#define CALIBRATE_CENTER(raw, center) \
        (abs((int32_t)(raw) - (int32_t)ADC_CENTER) <= CENTER_CALIBRATION_TOL) ? (raw) : ADC_CENTER

    left_x_center = CALIBRATE_CENTER(left_x_raw, ADC_CENTER);
    left_y_center = CALIBRATE_CENTER(left_y_raw, ADC_CENTER);
    right_x_center = CALIBRATE_CENTER(right_x_raw, ADC_CENTER);
    right_y_center = CALIBRATE_CENTER(right_y_raw, ADC_CENTER);

    // 初始化摇杆滤波数组（用校准后的中心值填充整个窗口）
    for(int i = 0; i < WIN_SIZE; i++) {
        filter_lx[i] = left_x_center;
        filter_ly[i] = left_y_center;
        filter_rx[i] = right_x_center;
        filter_ry[i] = right_y_center;
    }

    // 初始化摇杆结构体
    leftJoy.x_raw = left_x_center;
    leftJoy.y_raw = left_y_center;
    leftJoy.x_normalized = 0.0f;
    leftJoy.y_normalized = 0.0f;
    leftJoy.x_value = 0;
    leftJoy.y_value = 0;
    leftJoy.throttle = 0;

    rightJoy.x_raw = right_x_center;
    rightJoy.y_raw = right_y_center;
    rightJoy.x_normalized = 0.0f;
    rightJoy.y_normalized = 0.0f;
    rightJoy.x_value = 0;
    rightJoy.y_value = 0;
    rightJoy.throttle = 0;

    menu_hold_timer = 0;
    last_menu_direction = 0;
    menu_fast_mode = 0;

}

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
void Joy_Update(void)
{
    // 第一步：读取原始ADC值
    volatile uint16_t lx_raw = adc_buffer[0];
    volatile uint16_t ly_raw = adc_buffer[1];
    volatile uint16_t rx_raw = adc_buffer[2];
    volatile uint16_t ry_raw = adc_buffer[3];

     // DebugPrint("[Joy] Raw ADC: LX=%u, LY=%u, RX=%u, RY=%u\r\n",
     //           lx_raw, ly_raw, rx_raw, ry_raw);

    // 第二步：移动平均滤波
    uint16_t lx_filtered = SlidingFilter(filter_lx, lx_raw);
    uint16_t ly_filtered = SlidingFilter(filter_ly, ly_raw);
    uint16_t rx_filtered = SlidingFilter(filter_rx, rx_raw);
    uint16_t ry_filtered = SlidingFilter(filter_ry, ry_raw);

    // 保存滤波后的原始值
    leftJoy.x_raw = lx_filtered;
    leftJoy.y_raw = ly_filtered;
    rightJoy.x_raw = rx_filtered;
    rightJoy.y_raw = ry_filtered;

    // 转换为相对中心的偏移量
    int16_t left_x = (int16_t)lx_filtered - (int16_t)left_x_center;
    int16_t left_y = (int16_t)ly_filtered - (int16_t)left_y_center;
    int16_t right_x = (int16_t)rx_filtered - (int16_t)right_x_center;
    int16_t right_y = (int16_t)ry_filtered - (int16_t)right_y_center;

    // DebugPrint("[Joy] Filtered: LX=%u(%+d), LY=%u(%+d), RX=%u(%+d), RY=%u(%+d)\r\n",
    //       lx_filtered, left_x, ly_filtered, left_y,
    //       rx_filtered, right_x, ry_filtered, right_y);

    // 第三步：应用中心死区
    left_x = ApplyCenterDeadzone(left_x, CENTER_DEADZONE);
    left_y = ApplyCenterDeadzone(left_y, CENTER_DEADZONE);
    right_x = ApplyCenterDeadzone(right_x, CENTER_DEADZONE);
    right_y = ApplyCenterDeadzone(right_y, CENTER_DEADZONE);

    // 第四步：应用圆周死区并归一化
    float left_x_norm, left_y_norm;
    float right_x_norm, right_y_norm;

    ApplyRadialDeadzoneAndNormalize(left_x, left_y,
                                    ADC_MAX_RANGE, EDGE_DEADZONE,
                                    &left_x_norm, &left_y_norm);

    ApplyRadialDeadzoneAndNormalize(right_x, right_y,
                                    ADC_MAX_RANGE, EDGE_DEADZONE,
                                    &right_x_norm, &right_y_norm);

    // 保存归一化值（X轴向右为正，Y轴向上为正）
    leftJoy.x_normalized = -left_x_norm;
    leftJoy.y_normalized = left_y_norm;
    rightJoy.x_normalized = -right_x_norm;
    rightJoy.y_normalized = right_y_norm;

    // DebugPrint("[Joy] Normalized: LX=%.3f, LY=%.3f, RX=%.3f, RY=%.3f\r\n",
    //           leftJoy.x_normalized, leftJoy.y_normalized, rightJoy.x_normalized, rightJoy.y_normalized);

    // 转换为-100到100的整数值（用于菜单等）
    leftJoy.x_value = (int16_t)(-left_x_norm * 100.0f);
    leftJoy.y_value = (int16_t)(left_y_norm * 100.0f);
    rightJoy.x_value = (int16_t)(-right_x_norm * 100.0f);
    rightJoy.y_value = (int16_t)(right_y_norm * 100.0f);

    // 左摇杆Y轴油门：应用对数曲线
    float y_magnitude = fabsf(leftJoy.y_normalized);
    uint8_t is_negative = leftJoy.y_normalized < 0;
    float throttle_mapped = LogCurve(y_magnitude, is_negative);
    leftJoy.throttle = (int16_t)(throttle_mapped * 100.0f);

    // DebugPrint("Left Stick: X=%+6.2f (%+4d), Y=%+6.2f (%+4d), Throttle=%+4d\r\n",
    //           leftJoy.x_normalized, leftJoy.x_value,
    //           leftJoy.y_normalized, leftJoy.y_value,
    //           leftJoy.throttle);

    // DebugPrint("Right Stick: X=%+6.2f (%+4d), Y=%+6.2f (%+4d)\r\n",
    //           rightJoy.x_normalized, rightJoy.x_value,
    //           rightJoy.y_normalized, rightJoy.y_value);
}

/**
 * @brief 获取左摇杆油门值（对数曲线）
 * @return 油门值 -100到100（向上为正，向下为负）
 * @note 油门使用对数曲线，低速时更灵敏，高速时更平滑
 */
int16_t Joy_GetThrottle(void)
{
    return leftJoy.throttle;
}

/**
 * @brief 获取左摇杆完整数据
 * @return 左摇杆数据结构
 * @note 包含原始值、归一化值、整数值和油门值
 */
JoyStick_t Joy_GetLeftStick(void)
{
    return leftJoy;
}

/**
 * @brief 获取右摇杆完整数据
 * @return 右摇杆数据结构
 * @note 包含原始值、归一化值、整数值
 */
JoyStick_t Joy_GetRightStick(void)
{
    return rightJoy;
}

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
int8_t Joy_GetMenuControl(uint32_t delta_ms)
{
    int8_t menu_cmd = 0;
    int8_t current_direction = 0;

    // 判断当前方向（使用左摇杆X轴）
    if (leftJoy.x_value > MENU_THRESHOLD) {
        current_direction = 1;  // 右
    } else if (leftJoy.x_value < -MENU_THRESHOLD) {
        current_direction = -1;  // 左
    } else {
        current_direction = 0;  // 中间
    }

    // 方向改变时的处理
    if (current_direction != last_menu_direction) {
        if (last_menu_direction == 0 && current_direction != 0) {
            // 从中间位置开始划动
            menu_cmd = current_direction;  // 立即触发一次菜单移动
            menu_hold_timer = 0;
            menu_fast_mode = 0;
        } else if (current_direction == 0) {
            // 返回中间位置
            menu_hold_timer = 0;
            menu_fast_mode = 0;
        }
        last_menu_direction = current_direction;
    } else {
        // 方向保持不变
        if (current_direction != 0) {
            menu_hold_timer += delta_ms;

            // 保持超过1秒，进入快速模式
            if (menu_hold_timer >= MENU_FAST_TRIGGER_TIME) {
                if (!menu_fast_mode) {
                    menu_fast_mode = 1;
                }

                // 快速模式下定期触发
                static uint32_t fast_timer = 0;
                fast_timer += delta_ms;
                if (fast_timer >= MENU_FAST_INTERVAL) {
                    fast_timer = 0;
                    menu_cmd = current_direction * 2;  // 返回快速命令
                }
            }
        }
    }

    return menu_cmd;
}

/**
 * @brief 获取校准后的中心值（用于调试）
 * @param left_x 左摇杆X轴中心值输出（可为NULL）
 * @param left_y 左摇杆Y轴中心值输出（可为NULL）
 * @param right_x 右摇杆X轴中心值输出（可为NULL）
 * @param right_y 右摇杆Y轴中心值输出（可为NULL）
 */
void Joy_GetCenterValues(uint16_t *left_x, uint16_t *left_y, uint16_t *right_x, uint16_t *right_y)
{
    if (left_x) *left_x = left_x_center;
    if (left_y) *left_y = left_y_center;
    if (right_x) *right_x = right_x_center;
    if (right_y) *right_y = right_y_center;
}



