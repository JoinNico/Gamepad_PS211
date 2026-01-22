#include "debug.h"

#include <stdio.h>
#include <stdarg.h>
#include "cmsis_os.h"
#include "usart.h"


// 调试任务句柄
extern osThreadId debugTaskHandle;

/**
 * @brief 安全的串口发送函数（内部使用）
 */
static void debug_send_string(const char* str, uint16_t len)
{
    // 使用阻塞方式发送，超时100ms
    HAL_UART_Transmit(&huart1, (uint8_t*)str, len, 100);
}

/**
 * @brief 格式化调试信息并放入队列
 */
void DebugPrint(const char* format, ...)
{
    // 检查队列是否有效
    if (debugQueueHandle == NULL)
    {
        // 队列未初始化，直接输出
        static char buffer[128];
        va_list args;

        va_start(args, format);
        int len = vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        if (len > 0)
        {
            debug_send_string(buffer, len);
        }
        return;
    }

    debug_message_t msg;
    va_list args;

    // 格式化消息
    va_start(args, format);
    int len = vsnprintf(msg.message, sizeof(msg.message), format, args);
    va_end(args);

    if (len <= 0)
    {
        return; // 格式化失败
    }

    // 确保以空字符结尾
    if (len >= (int)sizeof(msg.message))
    {
        len = sizeof(msg.message) - 1;
    }

    msg.length = len;

    // 尝试将消息放入队列
    osStatus status = osMessagePut(debugQueueHandle, (uint32_t)&msg, 0);

    // 如果队列已满，等待10ms再试一次
    if (status != osOK)
    {
        status = osMessagePut(debugQueueHandle, (uint32_t)&msg, 10);

        // 如果仍然失败，直接输出（避免数据丢失）
        if (status != osOK)
        {
            debug_send_string("[WARN] Queue full, direct output: ", 35);
            debug_send_string(msg.message, len);
        }
    }
}