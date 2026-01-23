/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-04-28
 */
 
#include "elog.h"
#include <stdio.h>
#include "cmsis_os.h"
#include "usart.h"

/* 时间缓冲区 */
static char cur_system_time[32] = "";
static uint32_t last_tick = 0;
static uint32_t hour = 0, min = 0, sec = 0, ms = 0;

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
    ElogErrCode result = ELOG_NO_ERR;

    /* add your code here */
    
    return result;
}

/**
 * EasyLogger port deinitialize
 *
 */
void elog_port_deinit(void) {

    /* add your code here */

}

/**
 * @brief USART DMA传输完成回调函数
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        /* 释放DMA锁 */
        if (elog_dma_lockHandle != NULL) {
            osSemaphoreRelease(elog_dma_lockHandle);
        } else {
            // 可以添加调试信息，但注意不要在中断中调用阻塞函数
            // 可以设置一个标志位，在任务中检查
        }
    }
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
    /* 使用DMA发送，非阻塞 */
    HAL_UART_Transmit_DMA(&huart1, (uint8_t *)log, size);
    /* 等待DMA传输完成信号量（在回调函数中释放） */
    osSemaphoreWait(elog_dma_lockHandle, osWaitForever);

}

/**
 * output lock
 */
void elog_port_output_lock(void) {
    osSemaphoreWait(elog_lockHandle, osWaitForever);
}

/**
 * output unlock
 */
void elog_port_output_unlock(void) {
    osSemaphoreRelease(elog_lockHandle);
}

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
    uint32_t current_tick = osKernelSysTick();
    uint32_t elapsed = current_tick - last_tick;

    if (elapsed >= 1000 || last_tick == 0) {
        ms = current_tick % 1000;
        sec = (current_tick / 1000) % 60;
        min = (current_tick / (1000 * 60)) % 60;
        hour = (current_tick / (1000 * 60 * 60)) % 24;
        last_tick = current_tick;
    }

    snprintf(cur_system_time, sizeof(cur_system_time), "%02lu:%02lu:%02lu.%03lu",
             hour, min, sec, ms);

    return cur_system_time;
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
    
    /* add your code here */
    
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {
    /* FreeRTOS CMSIS v1中，可以使用osThreadGetId获取当前任务句柄 */
    osThreadId current_task = osThreadGetId();
    if (current_task != NULL) {
        /* 注意：CMSIS v1的osThreadGetId返回的是TaskHandle_t */
        return pcTaskGetName((TaskHandle_t)current_task);
    }
    return "";
}

/**
 * 异步输出通知函数
 */
void elog_async_output_notice(void) {
    osSemaphoreRelease(elog_asyncHandle);
}

/**
  * @brief 初始化EasyLogger系统
  */
void elog_init_system(void)
{
    /* 初始化EasyLogger */
    elog_init();

    /* 设置日志输出格式 */
    /* 断言：输出所有内容 */
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    /* 错误：输出级别、标签和时间 */
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    /* 警告：输出级别、标签和时间 */
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    /* 信息：输出级别、标签和时间 */
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    /* 调试：输出除了方法名之外的所有内容 */
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    /* 详细：输出除了方法名之外的所有内容 */
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);

    /* 设置日志过滤器（可选） */
    // elog_set_filter_lvl(ELOG_LVL_INFO); // 只显示INFO及以上级别
    // elog_set_filter_tag("System"); // 只显示特定tag

    /* 启动EasyLogger */
    elog_start();
}

/**
 * EasyLogger异步输出任务入口函数
 */
void StartELogTask(void const *argument) {
    elog_init_system();     // 初始化 EasyLogger 日志系统

    size_t get_log_size = 0;
#ifdef ELOG_ASYNC_LINE_OUTPUT
    static char poll_get_buf[ELOG_LINE_BUF_SIZE - 4];
#else
    static char poll_get_buf[ELOG_ASYNC_OUTPUT_BUF_SIZE - 4];
#endif

    for(;;) {
        //HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
        /* 等待日志信号 */
        osSemaphoreWait(elog_asyncHandle, osWaitForever);

        /* 轮询获取并输出日志 */
        while (1) {
#ifdef ELOG_ASYNC_LINE_OUTPUT
            get_log_size = elog_async_get_line_log(poll_get_buf, sizeof(poll_get_buf));
#else
            get_log_size = elog_async_get_log(poll_get_buf, sizeof(poll_get_buf));
#endif
            if (get_log_size) {
                elog_port_output(poll_get_buf, get_log_size);
            } else {
                break;
            }
        }
    }
}

