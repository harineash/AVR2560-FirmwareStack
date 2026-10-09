#include "ir.h"

void ir_init(IR_Config *ir)
{
    gpio_pinMode(ir->port, ir->pin, INPUT);
}

uint8_t ir_read(IR_Config *ir)
{
    return gpio_pinRead(ir->port, ir->pin);
}

uint8_t ir_detected(IR_Config *ir)
{
    if (gpio_pinRead(ir->port, ir->pin) == ir->active_level)
    {
        return 1;
    }

    return 0;
}
