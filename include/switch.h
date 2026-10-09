#ifndef SWITCH_H
#define SWITCH_H

#include "gpio.h"

typedef struct
{
    GPIO_Port port;
    uint8_t pin;
    uint8_t active_level;
    uint8_t pullup;
} Switch_Config;

void switch_init(Switch_Config *sw);
uint8_t switch_read(Switch_Config *sw);

#endif
