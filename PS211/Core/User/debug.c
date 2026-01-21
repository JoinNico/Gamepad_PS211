//
// Created by Administrator1 on 2026/1/21.
//

#include "debug.h"



/**
 * @brief 调试信息打印函数
 */
void DebugPrint(const char* format, ...)
{
    char buffer[128];
    va_list args;

    // 初始化参数列表
    va_start(args, format);

    // 安全地格式化字符串
    int len = vsnprintf(buffer, sizeof(buffer), format, args);

    // 结束参数列表
    va_end(args);

    // 确保字符串以空字符结尾
    if (len >= (int)sizeof(buffer)) {
        buffer[sizeof(buffer) - 1] = '\0';
    }

    // 使用HAL_UART直接输出，避免printf问题
    HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buffer, strlen(buffer));
}

// 添加简单的字符串输出函数
void DebugPrintString(const char* str)
{
    HAL_UART_Transmit_DMA(&huart1, (uint8_t*)str, strlen(str));
}