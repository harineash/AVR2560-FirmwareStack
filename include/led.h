#ifndef LED_H
#define LED_H

#include "gpio.h"

typedef struct
{
    GPIO_Port port;
    uint8_t pin;
    uint8_t active_level;
} LED_Config;

void led_init(LED_Config *led);
void led_on(LED_Config *led);
void led_off(LED_Config *led);
void led_toggle(LED_Config *led);

#endif
