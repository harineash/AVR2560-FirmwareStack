#ifndef TIMER_H
#define TIMER_H

#include "define.h"

typedef struct
{
    uint8_t timer;
    uint8_t mode;
    uint16_t prescaler;
    uint16_t compare;
} Timer_Config;

void timer_init(Timer_Config *timer);
void timer_start(Timer_Config *timer);
void timer_stop(Timer_Config *timer);
void timer_resetCount(Timer_Config *timer);
uint16_t timer_getCount(Timer_Config *timer);
void timer_delay_ms(uint16_t ms);
void timer_delay_us(uint16_t us);

#endif
