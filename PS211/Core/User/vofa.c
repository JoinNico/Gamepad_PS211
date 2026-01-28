/*
* vofa.c
 *
 *  Created on: 2023年5月23日
 *      Author: sundm
 */

#include "vofa.h"
#include "../../3rdParty/elog.h"
#include <string.h>

#include "usart.h"

const uint8_t tail[4] = {0x00, 0x00, 0x80, 0x7f};
uint8_t vofa_buffer[64]; // 可根据需要调整缓冲区大小

/*JustFloat - 浮点数转字节数组*/
static void float_turn_u8(float f, uint8_t *c) {
    FloatLongType data;
    data.fdata = f;

    for(uint8_t x = 0; x < 4; x++) {
        c[x] = (uint8_t)(data.ldata >> (x * 8));
    }
}

/**
 * @brief 使用EasyLogger发送VOFA格式数据
 * @param fdata 浮点数据数组
 * @param fdata_num 浮点数据数量
 */
void JustFloat_Send(float *fdata, uint16_t fdata_num)
{
    uint16_t total_bytes = fdata_num * 4 + 4;
    // 检查缓冲区大小
    if(total_bytes > sizeof(vofa_buffer)) {
        log_e("VOFA buffer too small! Need %d bytes, have %d",
               total_bytes, sizeof(vofa_buffer));
        return;
    }
    // 将浮点数转换为字节
    uint16_t offset = 0;
    for(uint16_t i = 0; i < fdata_num; i++) {
        float_turn_u8(fdata[i], &vofa_buffer[offset]);
        offset += 4;
    }

    // 添加VOFA帧尾
    vofa_buffer[offset++] = tail[0];
    vofa_buffer[offset++] = tail[1];
    vofa_buffer[offset++] = tail[2];
    vofa_buffer[offset++] = tail[3];

    // 方式1: 直接使用elog_raw输出原始数据（不添加任何格式）
    // 注意：elog_raw会直接发送原始数据，适合VOFA协议
    // elog_raw((const char *)vofa_buffer, total_bytes);

    // 方式2: 如果需要查看调试信息，可以使用以下方式（但会影响VOFA数据解析）
    // elog_raw("VOFA Data: ");
    // for(uint16_t i = 0; i < total_bytes; i++) {
    //     elog_raw("%02X ", vofa_buffer[i]);
    // }
    // elog_raw("\r\n");

    // 直接使用HAL库发送，不使用elog系统
    HAL_UART_Transmit_DMA(&huart1, vofa_buffer, total_bytes);

    // 简单等待（根据需要调整）
    while (HAL_UART_GetState(&huart1) == HAL_UART_STATE_BUSY_TX) {
        osDelay(1);
    }
}