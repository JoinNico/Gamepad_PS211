#include "debug.h"

#include <stdio.h>
#include <stdarg.h>
#include "cmsis_os.h"
#include "usart.h"
#include "../../3rdParty/elog.h"

/**
 * @brief 安全的串口发送函数（内部使用）
 */
static void debug_send_string(const char* str, uint16_t len)
{
    // 使用阻塞方式发送，超时100ms
    //HAL_UART_Transmit(&huart1, (uint8_t*)str, len, 100);
    HAL_UART_Transmit_DMA(&huart1, (uint8_t*)str, len);
}

/**
 * @brief 格式化调试信息并放入队列
 */
void DebugPrint(const char* format, ...)
{
    static char buffer[128];
    va_list args;

    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0) {
        debug_send_string(buffer, len);
    }
}



/**
 * @brief 输出系统状态信息
 */
void log_system_status(void)
{
    /* 输出任务状态 */
    log_i("System", "===== System Status =====");
    log_i("System", "Tick Count: %lu", HAL_GetTick());
    log_i("System", "Free Heap: %lu bytes", xPortGetFreeHeapSize());
    log_i("System", "Minimum Ever Free Heap: %lu bytes", xPortGetMinimumEverFreeHeapSize());

    /* 输出任务信息 */
    TaskHandle_t debugTask = (TaskHandle_t)debugTaskHandle;
    TaskHandle_t joystickTask = (TaskHandle_t)joystickTaskHandle;

    if (debugTask != NULL) {
        log_i("System", "DebugTask Stack High Water: %lu",
              uxTaskGetStackHighWaterMark(debugTask));
    }

    if (joystickTask != NULL) {
        log_i("System", "JoystickTask Stack High Water: %lu",
              uxTaskGetStackHighWaterMark(joystickTask));
    }

    log_i("System", "=========================");
}