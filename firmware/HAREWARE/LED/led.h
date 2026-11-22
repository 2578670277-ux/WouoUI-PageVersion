#ifndef __LED_H
#define __LED_H



#include "stm32f10x.h"

void LED_Init(void);

#define LED_ON (GPIOC->BSRR |= (1<<13))
#define LED_OFF (GPIOC->BSRR |= (1<<(13+18)))
#define LED_TOGGLE (GPIOC->ODR ^= (1<<(13)))

#endif


