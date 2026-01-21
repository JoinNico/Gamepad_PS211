#ifndef PS211_DEBUG_H
#define PS211_DEBUG_H

 #include <stdint.h>

// 定义调试消息结构体
typedef struct {
    char message[128];    // 消息内容
    uint16_t length;      // 消息长度
} debug_message_t;

// 调试模块初始化
void Debug_Init(void);

// 调试打印函数（格式化输出）
void DebugPrint(const char* format, ...);

#endif // PS211_DEBUG_H