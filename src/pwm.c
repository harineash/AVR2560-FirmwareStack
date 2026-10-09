#include "pwm.h"
#include "gpio.h"

static volatile uint8_t *reg8(uint16_t address)
{
    return (volatile uint8_t *)(uintptr_t)address;
}

static void pwm_pin(PWM_Config *pwm, GPIO_Port *port, uint8_t *pin)
{
    if (pwm->timer == TIMER0 && pwm->channel == PWM_CHANNEL_A)
    {
        *port = GPIO_PORTB;
        *pin = 7;
    }
    else if (pwm->timer == TIMER0 && pwm->channel == PWM_CHANNEL_B)
    {
        *port = GPIO_PORTG;
        *pin = 5;
    }
    else if (pwm->timer == TIMER2 && pwm->channel == PWM_CHANNEL_A)
    {
        *port = GPIO_PORTB;
        *pin = 4;
    }
    else
    {
        *port = GPIO_PORTH;
        *pin = 6;
    }
}

void pwm_init(PWM_Config *pwm)
{
    GPIO_Port port;
    uint8_t pin;

    pwm_pin(pwm, &port, &pin);
    gpio_pinMode(port, pin, OUTPUT);
    gpio_pinWrite(port, pin, LOW);

    if (pwm->timer == TIMER0)
    {
        *reg8(TCCR0A_ADDR) = (uint8_t)((1U << 1) | (1U << 0));
        *reg8(TCCR0B_ADDR) = 0;
    }
    else if (pwm->timer == TIMER2)
    {
        *reg8(TCCR2A_ADDR) = (uint8_t)((1U << 1) | (1U << 0));
        *reg8(TCCR2B_ADDR) = 0;
    }

    pwm_setDuty(pwm, pwm->duty);
}

void pwm_setDuty(PWM_Config *pwm, uint8_t duty)
{
    uint8_t compare;

    if (duty > 100)
    {
        duty = 100;
    }

    pwm->duty = duty;
    compare = (uint8_t)((255UL * duty) / 100UL);

    if (pwm->timer == TIMER0)
    {
        *reg8((pwm->channel == PWM_CHANNEL_A) ? OCR0A_ADDR : OCR0B_ADDR) = compare;
    }
    else if (pwm->timer == TIMER2)
    {
        *reg8((pwm->channel == PWM_CHANNEL_A) ? OCR2A_ADDR : OCR2B_ADDR) = compare;
    }
}

void pwm_start(PWM_Config *pwm)
{
    uint8_t com_bit = (pwm->channel == PWM_CHANNEL_A) ? 7 : 5;

    if (pwm->timer == TIMER0)
    {
        *reg8(TCCR0A_ADDR) |= (uint8_t)(1U << com_bit);
        *reg8(TCCR0B_ADDR) = (uint8_t)((*reg8(TCCR0B_ADDR) & 0xF8U) | 3U);
    }
    else if (pwm->timer == TIMER2)
    {
        *reg8(TCCR2A_ADDR) |= (uint8_t)(1U << com_bit);
        *reg8(TCCR2B_ADDR) = (uint8_t)((*reg8(TCCR2B_ADDR) & 0xF8U) | 3U);
    }
}

void pwm_stop(PWM_Config *pwm)
{
    GPIO_Port port;
    uint8_t pin;
    uint8_t com_mask = (pwm->channel == PWM_CHANNEL_A) ? (1U << 7) : (1U << 5);

    if (pwm->timer == TIMER0)
    {
        *reg8(TCCR0A_ADDR) &= (uint8_t)~com_mask;
        *reg8(TCCR0B_ADDR) &= 0xF8U;
    }
    else if (pwm->timer == TIMER2)
    {
        *reg8(TCCR2A_ADDR) &= (uint8_t)~com_mask;
        *reg8(TCCR2B_ADDR) &= 0xF8U;
    }

    pwm_pin(pwm, &port, &pin);
    gpio_pinWrite(port, pin, LOW);
}
