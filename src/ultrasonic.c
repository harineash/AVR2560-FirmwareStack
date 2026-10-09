#include "ultrasonic.h"

#define F_CPU 16000000UL

static volatile uint8_t *R(uint16_t a)
{
    return (volatile uint8_t *)a;
}

static void delay_us(uint16_t us)
{
    volatile uint16_t i;

    while (us--)
    {
        for (i = 0; i < (F_CPU / 4000000UL); i++)
        {
            __asm__ __volatile__("nop");
        }
    }
}

void ultrasonic_init(Ultrasonic_Config *u)
{
    gpio_pinMode(u->trigger_port, u->trigger_pin, OUTPUT);
    gpio_pinMode(u->echo_port, u->echo_pin, INPUT);
    gpio_pinWrite(u->trigger_port, u->trigger_pin, LOW);
}

uint16_t ultrasonic_read_cm(Ultrasonic_Config *u)
{
    uint32_t timeout = 0;
    uint32_t pulse = 0;

    gpio_pinWrite(u->trigger_port, u->trigger_pin, LOW);
    delay_us(2);
    gpio_pinWrite(u->trigger_port, u->trigger_pin, HIGH);
    delay_us(10);
    gpio_pinWrite(u->trigger_port, u->trigger_pin, LOW);

    while (gpio_pinRead(u->echo_port, u->echo_pin) == LOW)
    {
        if (++timeout > 60000UL)
        {
            return 0;
        }
    }

    while (gpio_pinRead(u->echo_port, u->echo_pin) == HIGH)
    {
        if (++pulse > 60000UL)
        {
            return 0;
        }
    }

    return (uint16_t)(pulse / 58UL);
}
