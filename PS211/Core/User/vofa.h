/*
* vofa.h
 *
 *  Created on: 2023年5月23日
 *      Author: sundm
 */

#ifndef _VOFA_H_
#define _VOFA_H_

#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
    float fdata;
    uint32_t ldata;
} FloatLongType;

void JustFloat_Send(float *fdata, uint16_t fdata_num);

#ifdef __cplusplus
}
#endif

#endif /* _VOFA_H_ */