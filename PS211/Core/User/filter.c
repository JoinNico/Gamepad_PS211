#include "filter.h"

const uint8_t filter_weight[WIN_SIZE] = {10, 15, 20, 25, 30, 40, 50, 60, 70, 80};

/**
  * @brief  滑动窗口加权滤波函数 (适用于uint16_t类型，如ADC原始值)
  * @param  array: 滑动窗口数组，需预先初始化
  * @param  new_value: 新采样的数据值
  * @retval 滤波后的结果 (uint16_t)
  */
uint16_t SlidingFilter(uint16_t array[WIN_SIZE], uint16_t new_value)
{
    // 窗口滑动：移除最旧数据，插入新数据
    for(int i = 0; i < WIN_SIZE - 1; i++) {
        array[i] = array[i + 1];
    }
    array[WIN_SIZE - 1] = new_value; // 修正：使用传入的参数 new_value

    // 加权求和（使用32位，防止溢出）
    uint32_t sum = 0;        // 使用32位整数进行累加
    uint16_t weight_sum = 0; // 权重的和（如果权重和是常量可预先计算）

    for(uint8_t i = 0; i < WIN_SIZE; i++) {
        // 假设 filter_weight 为 uint16_t 类型，且权重值不会过大
        sum += (uint32_t)filter_weight[i] * (uint32_t)array[i];
        weight_sum += filter_weight[i];
    }

    // 计算加权平均值（避免浮点运算）
    uint16_t result = 0;
    if(weight_sum != 0) {
        // 进行整数除法，实现四舍五入
        result = (uint16_t)((sum + (weight_sum / 2)) / weight_sum);
    }
    // 如果 weight_sum 为 0，返回 0 或可根据需求返回数组平均值等
    return result;
}

