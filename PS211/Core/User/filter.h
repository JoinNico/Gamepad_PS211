#ifndef USER_FILTER_H_
#define USER_FILTER_H_

#include <stdint.h>

#define WIN_SIZE 10

uint16_t SlidingFilter(uint16_t array[WIN_SIZE], uint16_t new_value);

#endif /* USER_FILTER_H_ */
