#ifndef DISPLAY_H
#define DISPLAY_H
#include  "stm32f10x.h"

extern const char JZCG[][16];
extern const char JaSuDu[][16];
extern const char TuoLuoYi[][16];
extern const char QiYaJi[][16];
extern const char CiLiJi[][16];
extern const char KaiQi[][16];
extern const char GuanBi[][16];
extern const char BATT[][16];
extern const char WiFi[][16];


void Display_init(void);
void Acc_Display(void);
void Display_Update(void);
void ConnectingDisplay(void);
#endif
