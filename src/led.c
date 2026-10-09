#include "led.h"

void led_init(LED_Config *led)
{
    gpio_pinMode(led->port, led->pin, OUTPUT);
    led_off(led);
}

void led_on(LED_Config *led)
{
    gpio_pinWrite(led->port, led->pin, led->active_level);
}

void led_off(LED_Config *led)
{
    gpio_pinWrite(led->port, led->pin, (uint8_t)!led->active_level);
}

void led_toggle(LED_Config *led)
{
    gpio_pinToggle(led->port, led->pin);
}
