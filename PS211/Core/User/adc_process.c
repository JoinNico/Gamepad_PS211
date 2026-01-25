//
// Created by Administrator1 on 2026/1/25.
//
// #include "adc_process.h"
// #include "adc.h"
// #include "tim.h"
//
// volatile static uint16_t adc_buffer[11] = { 0 };  // ADC DMA缓冲区，11个通道
//
// void ADC_Init(void) {
//     // TODO 1. 在RTOS启动前，使用 HAL_Delay 进行长时间阻塞采样，严重拖慢系统启动。
//     // TODO 2. 硬件初始化和软件校准耦合在一起，不符合实时系统分层设计。
//
//     // ADC 采样校准
//     HAL_ADCEx_Calibration_Start(&hadc1);
//
//     // 启动ADC DMA
//     if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, 11) != HAL_OK) {
//         Error_Handler();
//     }
//
//     // 启动 TIM8 以开始产生 100Hz TRGO 信号 -> 10ms 一次 ADC 转换
//     HAL_TIM_Base_Start(&htim8);
//
//     // 确保初始化校准数据干净
//     // HAL_Delay(5);
// }

#define LOG_TAG    "adc_process.c"
// #define LOG_LVL    ELOG_LVL_DEBUG
#include "adc_process.h"

#include <stdio.h>
#include <string.h>

#include "adc.h"
#include "tim.h"
#include "../../3rdParty/elog.h"

/* 私有全局变量 */
ADC_ProcessHandle_t hadcProc = {
    .new_data_ready = false,
    .battery_voltage = 0.0f
};

/* DMA双缓冲 - 由DMA自动循环写入 */
static uint16_t dma_buffer[ADC_PROCESS_DMA_BUFFER_SIZE];

/**
  * @brief  初始化ADC处理模块
  * @param  hadc: 已配置好的ADC句柄（需在CubeMX/其它地方配置好扫描序列）
  * @note   此函数会校准ADC，并启动DMA双缓冲传输。
  */
void ADC_PROCESS_Init(ADC_HandleTypeDef* hadc)
{
    /* 检查参数 */
    if (hadc == NULL) {
        log_e("The ADC handle is NULL");
        return;
    }

    /* 1. 校准ADC */
    if (HAL_ADCEx_Calibration_Start(hadc) != HAL_OK) {
        log_e("ADC calibration failed");
    }

    /* 2. 清零DMA缓冲区和ADC生数据保存区 */
    for (int i = 0; i < ADC_PROCESS_DMA_BUFFER_SIZE; i++) {
        dma_buffer[i] = 0;
    }
    for (int i = 0; i < ADC_PROCESS_NUM_CHANNELS; i++) {
        hadcProc.raw_buffer[i] = 0;
    }

    /* 3. 启动 DMA 循环模式、双缓冲 */
    if (HAL_ADC_Start_DMA(hadc,
                         (uint32_t*)dma_buffer,
                         ADC_PROCESS_DMA_BUFFER_SIZE) != HAL_OK) {
        /* 启动DMA失败 */
        log_e("The DMA of ADC failed to start");
        return;
    }

    // 4. 启动 TIM8 以开始产生 100Hz TRGO 信号 -> 10ms 一次 ADC 转换
    if (HAL_TIM_Base_Start(&htim8) != HAL_OK) {
        log_e("The tim base failed to start");
    }

    // 确保初始化校准数据干净
    HAL_Delay(5);

    /* 5. 更新状态 */
    hadcProc.new_data_ready = false;
}

/**
  * @brief  检查是否有新数据就绪
  * @retval true: 新数据可用； false: 数据未更新
  */
bool ADC_PROCESS_IsDataReady(void)
{
    bool ready = hadcProc.new_data_ready;
    return ready;
}

/**
  * @brief  清除数据就绪标志（通常在读取数据后调用）
  */
void ADC_PROCESS_ClearDataReadyFlag(void)
{
    hadcProc.new_data_ready = false;
}

/**
  * @brief  获取指定通道的最新原始ADC值
  * @param  ch: 通道索引（使用 ADC_ChannelIndex_t 枚举）
  * @retval 12位原始ADC值（0-4095）
  */
uint16_t ADC_PROCESS_GetRawValue(ADC_ChannelIndex_t ch)
{
    if (ch >= ADC_PROCESS_NUM_CHANNELS) {
        return 0;
    }
    return hadcProc.raw_buffer[ch];
}

/**
  * @brief  获取计算后的电池电压
  * @retval 电池电压，单位：伏特(V)
  * @note   假设电池电压通过分压电阻连接至ADC。
  *         例如：分压比为 2:1 (R1=100k, R2=100k)，则实际电压 = ADC值 * 3.3V / 4095 * (R1+R2)/R2
  */
float ADC_PROCESS_GetBatteryVoltage(void)
{
    /* 请根据你的实际分压电路修改以下参数 */
    const float VREF = 3.3f;          // ADC参考电压（与你给VDDA的电压一致）
    const float DIVIDER_RATIO = 5.7f; // 分压比
    const uint16_t adc_max = 4095;    // 12位ADC最大值

    uint16_t raw = hadcProc.raw_buffer[ADC_CH_BAT_VOLT];
    // log_i("The battery raw voltage is %d",raw);
    hadcProc.battery_voltage = ((float)raw * VREF / (float)adc_max) * DIVIDER_RATIO;
    // log_i("The battery voltage is %.2f",hadcProc.battery_voltage);
    return hadcProc.battery_voltage;
}

/**
 * @brief Simplified version - only prints raw values
 */
void adc_print_raw_buffer_simple(const char* buffer_name,
                                 uint16_t* buffer,
                                 uint16_t size) {

    log_i("[ADC] %s [%d samples]:", buffer_name, size);

    char data_str[256] = "";
    char temp[16];
    int pos = 0;

    // Print a few samples per line to avoid long logs
    for (int i = 0; i < size; i++) {
        snprintf(temp, sizeof(temp), "%u", buffer[i]);

        // Check if there's enough space
        if (pos + strlen(temp) + 2 < sizeof(data_str)) {
            if (i > 0) {
                data_str[pos++] = ',';
                data_str[pos++] = ' ';
            }
            strcpy(&data_str[pos], temp);
            pos += strlen(temp);
        }

        // New line every 4 samples
        if ((i + 1) % 4 == 0 || i == size - 1) {
            log_i("[ADC]   [%d-%d]: %s",
                  i - (i % 4), i, data_str);
            pos = 0;
            data_str[0] = '\0';
        }
    }
}
/**
  * @brief  处理新到达的DMA缓冲区数据
  * @param  buffer: 指向新数据缓冲区的指针
  */
static void ADC_ProcessNewBuffer(uint16_t* buffer)
{
    /* 将数据复制到供外部读取的缓冲区 */
    for (int i = 0; i < ADC_PROCESS_NUM_CHANNELS; i++) {
        hadcProc.raw_buffer[i] = buffer[i];
    }

    /* 更新状态 */
    hadcProc.new_data_ready = true;
}

/**
  * @brief  ADC半转换完成回调（DMA前半传输完成）
  */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
    /* 处理DMA缓冲区前半部分（即第一组数据）*/
    ADC_ProcessNewBuffer(&dma_buffer[0]);
}

/**
  * @brief  ADC转换完成回调（DMA后半传输完成）
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    /* 处理DMA缓冲区后半部分（即第二组数据）*/
    ADC_ProcessNewBuffer(&dma_buffer[ADC_PROCESS_NUM_CHANNELS]);
}




