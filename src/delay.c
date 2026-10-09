#include "delay.h"
#include "timer.h"

void delay_ms(uint16_t ms)
{
    timer_delay_ms(ms);
}

void delay_s(uint16_t seconds)
{
    while (seconds--)
    {
        timer_delay_ms(1000);
    }
}
