#include "ultrasonic.h"
#include "timer.h"

static volatile uint8_t *reg8(uint16_t address)
{
    return (volatile uint8_t *)(uintptr_t)address;
}

static volatile uint16_t *reg16(uint16_t address)
{
    return (volatile uint16_t *)(uintptr_t)address;
}

static void timer1_measurement_start(void)
{
    *reg8(TCCR1A_ADDR) = 0;
    *reg8(TCCR1B_ADDR) = 0;
    *reg16(TCNT1_ADDR) = 0;
    *reg8(TCCR1B_ADDR) = 2;
}

static void timer1_measurement_stop(void)
{
    *reg8(TCCR1B_ADDR) = 0;
}

void ultrasonic_init(Ultrasonic_Config *sensor)
{
    gpio_pinMode(sensor->trigger_port, sensor->trigger_pin, OUTPUT);
    gpio_pinMode(sensor->echo_port, sensor->echo_pin, INPUT);
    gpio_pinWrite(sensor->trigger_port, sensor->trigger_pin, LOW);
}

uint16_t ultrasonic_read_cm(Ultrasonic_Config *sensor)
{
    uint16_t start;
    uint16_t pulse_ticks;

    gpio_pinWrite(sensor->trigger_port, sensor->trigger_pin, LOW);
    timer_delay_us(2);
    gpio_pinWrite(sensor->trigger_port, sensor->trigger_pin, HIGH);
    timer_delay_us(10);
    gpio_pinWrite(sensor->trigger_port, sensor->trigger_pin, LOW);

    timer1_measurement_start();

    while (gpio_pinRead(sensor->echo_port, sensor->echo_pin) == LOW)
    {
        if (*reg16(TCNT1_ADDR) >= 60000U)
        {
            timer1_measurement_stop();
            return 0;
        }
    }

    start = *reg16(TCNT1_ADDR);

    while (gpio_pinRead(sensor->echo_port, sensor->echo_pin) == HIGH)
    {
        pulse_ticks = (uint16_t)(*reg16(TCNT1_ADDR) - start);
        if (pulse_ticks >= 60000U)
        {
            timer1_measurement_stop();
            return 0;
        }
    }

    pulse_ticks = (uint16_t)(*reg16(TCNT1_ADDR) - start);
    timer1_measurement_stop();

    return (uint16_t)(pulse_ticks / 116U);
}
