#include "pwm.h"

static volatile uint8_t *R(uint16_t a)
{
    return (volatile uint8_t *)a;
}

void pwm_init(PWM_Config *p)
{
    pwm_setDuty(p, p->duty);

    if (p->timer == TIMER0)
    {
        if (p->channel == PWM_CHANNEL_A)
        {
            *R(TCCR0A_ADDR) |= (1 << 7) | (1 << 1);
        }
        else
        {
            *R(TCCR0A_ADDR) |= (1 << 5) | (1 << 1);
        }
    }
    else if (p->timer == TIMER2)
    {
        if (p->channel == PWM_CHANNEL_A)
        {
            *R(TCCR2A_ADDR) |= (1 << 7) | (1 << 1);
        }
        else
        {
            *R(TCCR2A_ADDR) |= (1 << 5) | (1 << 1);
        }
    }
}

void pwm_setDuty(PWM_Config *p, uint8_t d)
{
    if (d > 100)
    {
        d = 100;
    }

    p->duty = d;

    uint8_t v = (uint8_t)((255UL * d) / 100UL);

    if (p->timer == TIMER0)
    {
        if (p->channel == PWM_CHANNEL_A)
        {
            *R(OCR0A_ADDR) = v;
        }
        else
        {
            *R(OCR0B_ADDR) = v;
        }
    }
    else if (p->timer == TIMER2)
    {
        if (p->channel == PWM_CHANNEL_A)
        {
            *R(OCR2A_ADDR) = v;
        }
        else
        {
            *R(OCR2B_ADDR) = v;
        }
    }
}

void pwm_start(PWM_Config *p)
{
    if (p->timer == TIMER0)
    {
        *R(TCCR0B_ADDR) = 3;
    }
    else if (p->timer == TIMER2)
    {
        *R(TCCR2B_ADDR) = 3;
    }
}

void pwm_stop(PWM_Config *p)
{
    if (p->timer == TIMER0)
    {
        *R(TCCR0B_ADDR) = 0;
    }
    else if (p->timer == TIMER2)
    {
        *R(TCCR2B_ADDR) = 0;
    }
}
