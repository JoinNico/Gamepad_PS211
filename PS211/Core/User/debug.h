//
// Created by Administrator1 on 2026/1/21.
//

#ifndef PS211_DEBUG_H
#define PS211_DEBUG_H

#include <stdlib.h>
#include <stdio.h>  // 添加stdio.h以使用printf
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <string.h>
#include "usart.h"

void DebugPrint(const char* format, ...);
void DebugPrintString(const char* str);
#endif //PS211_DEBUG_H