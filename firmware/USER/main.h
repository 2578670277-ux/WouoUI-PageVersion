#ifndef __MAIN_H__
#define __MAIN_H__


#include "stm32f10x.h"

enum {
    task_10ms_index = 0x00,
    task_100ms_index,
    task_500ms_index,
    TASK_MAX_SIZE,
};
extern uint8_t task_flag[TASK_MAX_SIZE];
extern uint32_t delay_tick;

void delay_ms(uint32_t delay_ms);


#endif

