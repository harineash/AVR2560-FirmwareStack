#include "switch.h"

void switch_init(Switch_Config *sw)
{
    gpio_pinMode(sw->port, sw->pin, INPUT);

    if (sw->pullup)
    {
        gpio_pinWrite(sw->port, sw->pin, HIGH);
    }
}

uint8_t switch_read(Switch_Config *sw)
{
    return (uint8_t)(gpio_pinRead(sw->port, sw->pin) == sw->active_level);
}
