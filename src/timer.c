#include "timer.h"

static volatile uint8_t *reg8(uint16_t address)
{
    return (volatile uint8_t *)(uintptr_t)address;
}

static volatile uint16_t *reg16(uint16_t address)
{
    return (volatile uint16_t *)(uintptr_t)address;
}

static uint8_t timer01_prescaler_bits(uint16_t prescaler)
{
    switch (prescaler)
    {
        case 1:    return 1;
        case 8:    return 2;
        case 64:   return 3;
        case 256:  return 4;
        case 1024: return 5;
        default:   return 0;
    }
}

static uint8_t timer2_prescaler_bits(uint16_t prescaler)
{
    switch (prescaler)
    {
        case 1:    return 1;
        case 8:    return 2;
        case 32:   return 3;
        case 64:   return 4;
        case 128:  return 5;
        case 256:  return 6;
        case 1024: return 7;
        default:   return 0;
    }
}

void timer_init(Timer_Config *timer)
{
    if (timer->timer == TIMER0)
    {
        *reg8(TCCR0A_ADDR) = (timer->mode == TIMER_CTC) ? (1U << 1) : 0;
        *reg8(TCCR0B_ADDR) = 0;
        *reg8(TCNT0_ADDR) = 0;
        *reg8(OCR0A_ADDR) = (uint8_t)timer->compare;
    }
    else if (timer->timer == TIMER1)
    {
        *reg8(TCCR1A_ADDR) = 0;
        *reg8(TCCR1B_ADDR) = (timer->mode == TIMER_CTC) ? (1U << 3) : 0;
        *reg16(TCNT1_ADDR) = 0;
        *reg16(OCR1A_ADDR) = timer->compare;
    }
    else if (timer->timer == TIMER2)
    {
        *reg8(TCCR2A_ADDR) = (timer->mode == TIMER_CTC) ? (1U << 1) : 0;
        *reg8(TCCR2B_ADDR) = 0;
        *reg8(TCNT2_ADDR) = 0;
        *reg8(OCR2A_ADDR) = (uint8_t)timer->compare;
    }
}

void timer_start(Timer_Config *timer)
{
    if (timer->timer == TIMER0)
    {
        *reg8(TCCR0B_ADDR) = (uint8_t)((*reg8(TCCR0B_ADDR) & 0xF8U) |
                                      timer01_prescaler_bits(timer->prescaler));
    }
    else if (timer->timer == TIMER1)
    {
        *reg8(TCCR1B_ADDR) = (uint8_t)((*reg8(TCCR1B_ADDR) & 0xF8U) |
                                      timer01_prescaler_bits(timer->prescaler));
    }
    else if (timer->timer == TIMER2)
    {
        *reg8(TCCR2B_ADDR) = (uint8_t)((*reg8(TCCR2B_ADDR) & 0xF8U) |
                                      timer2_prescaler_bits(timer->prescaler));
    }
}

void timer_stop(Timer_Config *timer)
{
    if (timer->timer == TIMER0)
    {
        *reg8(TCCR0B_ADDR) &= 0xF8U;
    }
    else if (timer->timer == TIMER1)
    {
        *reg8(TCCR1B_ADDR) &= 0xF8U;
    }
    else if (timer->timer == TIMER2)
    {
        *reg8(TCCR2B_ADDR) &= 0xF8U;
    }
}

void timer_resetCount(Timer_Config *timer)
{
    if (timer->timer == TIMER0)
    {
        *reg8(TCNT0_ADDR) = 0;
    }
    else if (timer->timer == TIMER1)
    {
        *reg16(TCNT1_ADDR) = 0;
    }
    else if (timer->timer == TIMER2)
    {
        *reg8(TCNT2_ADDR) = 0;
    }
}

uint16_t timer_getCount(Timer_Config *timer)
{
    if (timer->timer == TIMER0)
    {
        return *reg8(TCNT0_ADDR);
    }
    if (timer->timer == TIMER1)
    {
        return *reg16(TCNT1_ADDR);
    }
    if (timer->timer == TIMER2)
    {
        return *reg8(TCNT2_ADDR);
    }

    return 0;
}

static void timer1_wait(uint16_t top, uint8_t prescaler_bits)
{
    *reg8(TCCR1A_ADDR) = 0;
    *reg8(TCCR1B_ADDR) = 0;
    *reg16(TCNT1_ADDR) = 0;
    *reg16(OCR1A_ADDR) = top;
    *reg8(TIFR1_ADDR) = (1U << 1);
    *reg8(TCCR1B_ADDR) = (uint8_t)((1U << 3) | prescaler_bits);

    while ((*reg8(TIFR1_ADDR) & (1U << 1)) == 0)
    {
    }

    *reg8(TCCR1B_ADDR) = 0;
}

void timer_delay_ms(uint16_t ms)
{
    while (ms--)
    {
        timer1_wait(249, timer01_prescaler_bits(PRESCALER_64));
    }
}

void timer_delay_us(uint16_t us)
{
    while (us)
    {
        uint16_t chunk = (us > 32767U) ? 32767U : us;
        timer1_wait((uint16_t)(chunk * 2U - 1U),
                    timer01_prescaler_bits(PRESCALER_8));
        us = (uint16_t)(us - chunk);
    }
}
